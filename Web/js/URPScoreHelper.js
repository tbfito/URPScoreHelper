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
		url: "captcha.fcgi",
		dataType: "text",
		cache: false,
		beforeSend: function() {
			obj.src = "img/loading.gif";
		},
		success: function(data) {
			if(data == "LOGGED-IN")
			{
				$.toast("已登录过，跳转中", "text");
				ajax_page("main.fcgi");
			}
			else if(data == "REQUEST-FAILED")
			{
				$.toast("学校后端连接失败", "text");
				obj.src = "img/refresh.png";
			}
			else
			{
				obj.src = data;
				ocr_captcha(data);
			}
		},
		error: function() {
			obj.src = "img/refresh.png";
		}
	})
}
function get_avatar() {
	var obj = document.getElementsByClassName("i_user-photo")[0];
	if(obj == undefined)
		return;
	$.ajax({
		type: "GET",
		url: "avatar.fcgi",
		dataType: "text",
		cache: false,
		success: function(data) {
			if(data == "LOGGED-OUT")
			{
				$.toast("尚未登录，请登录", "text");
				ajax_page("index.fcgi?act=logout");
			}
			else
			{
				obj.src = data;
				$(".swiper-container").swiper({loop:true,height:160,autoplay:5000,observer:true,observeParents:true,lazyLoading:true});
			}
		},
		error: function() {
			$.toast("网络错误，请重试", "text");
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
		ajax_page("index.fcgi?act=logout");
	}, function() {
	});
}
function unlink(id) {
	$.confirm("确定要解除与微信帐号的绑定吗？", function() {
		$.toptip("正在解绑...", 2000, 'success');
		ajax_page("OAuth2.fcgi?act=link&release=" + id);
	}, function() {
	});
}
function change_password() {
	var r1 = document.getElementById("i_xhs").value;
	var r2 = document.getElementById("i_qr").value;
	if(r1=="")
	{
		$.toast("请输入新密码","cancel");
		return false;
	}
	if(r2=="" || r2 != r1)
	{
		$.toast("输入不一致","cancel");
		return false;
	}
	var patrn=/^(\w){6,12}$/;
	if (!patrn.exec(r1)){
		$.toast("只能输入6-12个字母、数字、下划线", "text");
		return false;
	}
	if(!document.getElementById("i_chk").checked){
		$.toast("滑动右边开关来确认输入无误 :)", "text");
		return false;
	}
	$.toptip("正在修改...", 2000, 'success');
	ajax_submit(null);
	return true;
}
function adjust_form(href) {
	if(href != null) {
		if(href.slice(-1) == "/" || href.slice(-10) == "index.fcgi" || href.slice(-21) == "index.fcgi?act=logout" || href.slice(-19) == "changePassword.fcgi")
		{
			return;
		}
	}
	else if(window.location.pathname.slice(-1) == "/")
	{
		return;
	}
	r1 = document.getElementById("i_xh");
	r2 = document.getElementById("i_mm");
	if(r1 == undefined || r2 == undefined) {
		return;
	}
	if(r1.value != "") {
		document.getElementsByClassName("weui-cell")[0].style.display = "none";
	}
	if(r2.value != "") {
		document.getElementsByClassName("weui-cell")[1].style.display = "none";
	}
}
function init(href){
	adjust_form(href);
	get_captcha();
	get_avatar();
	card_animation();
	var r1 = document.getElementById("i_xh");
	var r2 = document.getElementById("i_mm");
	var r3 = document.getElementById("i_yzm");
	var r5 = document.getElementById("i_jxpg");
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
	if(r5 != undefined)
	{
		getcharnum();
	}
	$("#i_submit").on("click", function(e) {
		var r4 = document.getElementById("weuiAgree");
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
			$.toast("请输入验证码","cancel");
			return false;
		}
		if(r4 != undefined && !r4.checked)
		{
			$.toast("请同意条款哦","cancel");
			return false;
		}
		if(r5 != undefined && r5.value == "")
		{
			$.toast("请写主观评价","cancel");
			return false;
		}
		$.toptip("正在提交，请稍后...", 2000, 'success');
		ajax_submit(null);
		return true;
	});
	try {
		if (typeof(eval(init_test_list)) == "function") {
			init_test_list();
		}
	} catch(e) {}
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
function ajax_submit(mydata) {
	$(".loading").show();
	var href = $("#ajax_submit").attr("data-ajax-submit");
	if (href != undefined) {
		$.ajax({
			url: href,
			type: "POST",
			contentType: "application/x-www-form-urlencoded",
			dataType: "text",
			data: mydata == null ? $('#ajax_submit').serialize() : mydata,
			beforeSend: function(request) {
				request.setRequestHeader("X-Ajax-Request", "1");
			},
			error: function(request) {
				$(".loading").hide();
				$.toast("请求失败，请重试","text");
				ajax_page("/");
			},
			success: function(data) {
				$("#container").html(data);
				init(href);
				$(".loading").hide();
			}
		});
	}
}
function query_tests() {
	var r1 = document.getElementById("tests").value;
	if(r1 == undefined || r1 == "" || r1 == "点击这里选择...")
	{
		$.toast("请选择一场考试！","text");
		return false;
	}
	var r2 = $("#tests").attr("data-values");
	if(r2 == undefined || r2 == "")
	{
		$.toast("未知错误，请重试","text");
		return false;
	}
	$.toptip("正在查询...", 2000, 'success');
	ajax_submit("tests=" + encodeURIComponent(r1 + "|" + r2));
	return true;
}
function card_animation() {
	var obj = document.getElementsByClassName("i_user-photo")[0];
	if(obj == undefined)
		return;
	$('.i_vision').parallax();
}
$(document).ready(init(null));