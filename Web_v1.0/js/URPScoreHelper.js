function createxmlHttpRequest() {
	if (window.ActiveXObject) {
		return new ActiveXObject("Microsoft.XMLHTTP");
	} else if (window.XMLHttpRequest) {
		return new XMLHttpRequest();
	}
}

function convertData(data) {
	if (typeof data === 'object') {
		var convertResult = "";
		for (var c in data) {
			convertResult += c + "=" + data[c] + "&";
		}
		convertResult = convertResult.substring(0, convertResult.length - 1);
		return convertResult;
	} else {
		return data;
	}
}

function ajax() {
	var ajaxData = {
		type: arguments[0].type || "GET",
		url: arguments[0].url || "",
		async: arguments[0].async || "true",
		data: arguments[0].data || null,
		dataType: arguments[0].dataType || "text",
		contentType: arguments[0].contentType || "application/x-www-form-urlencoded",
		beforeSend: arguments[0].beforeSend ||
		function() {},
		success: arguments[0].success ||
		function() {},
		error: arguments[0].error ||
		function() {}
	}
	ajaxData.beforeSend();
	var xhr = createxmlHttpRequest();
	xhr.responseType = ajaxData.dataType;
	xhr.open(ajaxData.type, ajaxData.url, ajaxData.async);
	xhr.setRequestHeader("Content-Type", ajaxData.contentType);
	xhr.send(convertData(ajaxData.data));
	xhr.onreadystatechange = function() {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				ajaxData.success(xhr.responseText)
			} else {
				ajaxData.error(xhr.responseText)
			}
		}
	}
}

function get_captcha() {
	ajax({
		type: "GET",
		url: "captcha.cgi",
		beforeSend: function() {
			document.getElementById("login_captcha").src = "img/loading.gif";
		},
		success: function(msg) {
			document.getElementById("login_captcha").src = msg;
			ocr_captcha(msg);
		},
		error: function(msg) {
			document.getElementById("login_captcha").src = "img/refresh.png";
			$.toast(msg);
		}
	})
}

function ocr_captcha(dataURI) {
	ajax({
		type: "POST",
		url: "https://api.iedon.com/captcha_ocr/ocr.php",
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
				msg.replace("l", "1");
				msg.replace("I", "1");
				msg.replace("O", "0");
				document.getElementById("i_yzm").value = msg;
			}
		},
	})
}

function show_loading() {
	$.showPreloader('请稍候');
	setTimeout(function () {
		$.hidePreloader();
	}, 10000);
}
function getQueryString(name) {
	var i;
    var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", i); // 匹配目标参数
    var result = window.location.search.substr(1).match(reg);  // 对querystring匹配目标参数
    if (result != null) {
        return decodeURIComponent(result[2]);
    } else {
        return null;
    }
}
$(function () {
	$(document).on("click", "a:not(.except)", function(e) {
		show_loading();
	});
		$(document).on("click", "#i_submit", function(e) {
				document.oncontextmenu=new Function("event.returnValue=false;");
				   var r1 = document.getElementById("i_xh").value;
				   var r2 = document.getElementById("i_mm").value;
				   var r3 = document.getElementById("i_yzm").value;
				if(r1=="")
				{
					$.toast("(⊙o⊙) 学号还没输呢。");
					return false;
				}
				if(r2=="")
				{
					$.toast("(⊙o⊙) 密码还没输呢。");
					return false;
				}
				if(r3=="")
				{
					$.toast("(⊙o⊙) 验证码还没输呢。");
					return false;
				}
				show_logging();
				function show_logging() {
					$.showPreloader('O(∩_∩)O 正在登录');
					setTimeout(function () {
						$.hidePreloader();
					}, 10000);
				 }
		});
		$.init();
});
$.config = {router: false}
function show_about() {
	var r1 = document.getElementById("about").value;
	$.alert(r1);
}
function check() {
	var r1 = document.getElementById("i_xhs").value;
	if(r1=="")
	{
		$.toast("(⊙o⊙) 总得输入些什么吧？");
		return false;
	}
	$.showPreloader('O(∩_∩)O 请稍候');
	setTimeout(function () {
		$.hidePreloader();
	}, 10000);
	return true;
}