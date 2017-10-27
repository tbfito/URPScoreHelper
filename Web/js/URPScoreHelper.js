function ocr_captcha(dataURI) {
	$.ajax({
		type: "POST",
		url: "https://api.iedon.com/captcha",
		dataType: "text",
		cache: false,
		data:{"pic":dataURI}, 
		success: function(msg) {
			if(msg != null && msg.length != 0) {
				if(msg == '__ERROR')
				{
					get_captcha();
					return;
				}
				var patrn=/^(\w){4,4}$/;
				if (!patrn.exec(msg))
				{
					get_captcha();
					return;
				}
				document.getElementById("i_yzm").value = msg;
			}
		},
	})
}
function get_captcha() {
	var obj = document.getElementById("login_captcha");
	if(obj == undefined)
		return;
	$.ajax({
		type: "GET",
		url: "/captcha.fcgi",
		dataType: "text",
		cache: false,
		beforeSend: function() {
			obj.src = "/img/loading.gif";
		},
		success: function(data) {
			if(data == "LOGGED-IN")
			{
				$.toast("已登录过, 正在跳转", "text");
				ajax_page("/main.fcgi");
			}
			else if(data == "REQUEST-FAILED")
			{
				$.toast("验证码失踪了！学院系统可能发生故障", "text");
				obj.src = "/img/refresh.png";
			}
			else
			{
				obj.src = data;
				ocr_captcha(data);
			}
		},
		error: function() {
			obj.src = "/img/refresh.png";
		}
	})
}
function get_avatar() {
	var obj = document.getElementsByClassName("i_user-photo")[0];
	if(obj == undefined)
		return;
	$.ajax({
		type: "GET",
		url: "/avatar.fcgi",
		dataType: "text",
		cache: true,
		success: function(data) {
			if(data == "LOGGED-OUT")
			{
				$.toast("尚未登录，请登录！", "text");
				ajax_page("/index.fcgi?act=logout");
			}
			else
			{
				obj.src = data;
				$(".swiper-container").swiper({loop:true,height:160,autoplay:5000,observer:true,observeParents:true,lazyLoading:true});
			}
		},
		error: function() {
			$.toast("照片飘到火星啦，请刷新重试。", "text");
		}
	})
}
function autoreset() {
	obj1 = document.getElementById("i_xh");
	obj2 = document.getElementById("i_mm");
	obj1.value = "";
	obj2.value = "";
	obj1.focus();
}
function autofill() {
	obj = document.getElementById("i_xh");
	if(obj.value=="")
	{
		$.toast("学号还没输呢","cancel");
		return false;
	}
	$.toptip("已自动填入密码，密码与学号相同方可用", 3000, 'success');
	document.getElementById("i_mm").value = obj.value;
	document.getElementById("i_yzm").focus();
}
function checkRows() {
	var rows = document.getElementById("i_xh").value.split(/\r?\n|\r/).length;
	document.getElementById("i_xhhs").innerHTML = rows;
}
function getcharnum() {
	var nums = document.getElementById("i_jxpg").value.length;
	document.getElementById("i_xhhs").innerHTML = nums;
}
function logout() {
	$.confirm("确认要退出系统吗？", function() {
		$.toptip("正在登出...", 2000, 'success');
		ajax_page("/index.fcgi?act=logout");
	}, function() {
	});
}
function releaseAssoc(id) {
	$.confirm("确定要解除学号与QQ号的关联吗？", function() {
		$.toptip("正在解绑...", 2000, 'success');
		ajax_page("/OAuth2Assoc.fcgi?release=" + id);
	}, function() {
	});
}
function check_password() {
	var r1 = document.getElementById("i_xhs").value;
	if(r1=="")
	{
		$.toast("请输入新密码","cancel");
		return false;
	}
	var patrn=/^(\w){6,12}$/;
	if (!patrn.exec(r1)){
		$.toast("只能输入6-12个字母、数字、下划线", "text");
		return false;
	}
	if(!document.getElementById("i_chk").checked){
		$.toast("滑动右边开关来确认输入无误 :-)", "text");
		return false;
	}
	$.toptip("正在修改...", 5000, 'success');
	$(".loading").show();
	return true;
}
function adjust_form(href) {
	if(href != null) {
		if(href == "/" || href == "/index.fcgi?act=logout")
		{
			return;
		}
	}
	else if(window.location.pathname == "/")
	{
		return;
	}
	r1 = document.getElementById("i_xh");
	r2 = document.getElementById("i_mm");
	if(r1 == undefined || r2 == undefined) {
		return;
	}
	if(r1.value != "") {
		document.getElementsByClassName("weui-cell weui-cell_vcode")[0].style.display = "none";
	}
	if(r2.value != "") {
		document.getElementsByClassName("weui-cell weui-cell_vcode")[1].style.display = "none";
	}
}
function init(href){
	adjust_form(href);
	get_captcha();
	get_avatar();
	var r1 = document.getElementById("i_xh");
	var r2 = document.getElementById("i_mm");
	var r3 = document.getElementById("i_yzm");
	if(r1 != undefined && r1.value == "")
	{
		r1.focus();
	}
	if(r1 != undefined && r1.value != "" && r2 != undefined && r2.value == "")
	{
		r2.focus();
	}
	if(r1 != undefined && r1.value != "" && r2 != undefined && r2.value != "")
	{
		r3.focus();
	}
	$("#i_submit").on("click", function(e) {
		var r4 = document.getElementById("weuiAgree");
		var r5 = document.getElementById("i_jxpj");
		if(r1 != undefined && r1.value == "")
		{
			$.toast("学号还没输呢","cancel");
			return false;
		}
		if(r2 != undefined && r2.value == "")
		{
			$.toast("密码还没输呢","cancel");
			return false;
		}
		if(r3 != undefined && r3.value == "")
		{
			$.toast("验证码还没输呢","cancel");
			return false;
		}
		if(r4 != undefined && !r4.checked)
		{
			$.toast("请同意条款哦","cancel");
			return false;
		}
		if(r5 != undefined && r5.value == "")
		{
			$.toast("没写主观评价啊","cancel");
			return false;
		}
		$.toptip("正在提交，请稍后...", 5000, 'success');
		$(".loading").show();
		return true;
	});
	$(".loading").hide();
	$("a").not("#logout").not("#no_ajax").on("click", function(e) {
		var href = $(this).attr("href");
		e.preventDefault();
		ajax_page(href);
	});
}
function ajax_page(href) {
	$(".loading").show();
	if (href != undefined) {
		$.ajax({
			url: href,
			type: "GET",
			dataType: "text",
			beforeSend: function(request) {
				request.setRequestHeader("X-Ajax-Request", "1");
			},
			error: function(request) {
				window.location.href = href;
			},
			success: function(data) {
				$("#container").html(data);
				init(href);
				$(".loading").hide();
			}
		});
	}
}
$(document).ready(init(null));