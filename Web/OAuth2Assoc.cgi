<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-share"></span>&nbsp;&nbsp;<a class="except" onclick="return show_about();">关于</a></button>
		<button class="button button-link button-nav pull-right"><a href="Comments.html">吐槽</a>&nbsp;&nbsp;<span class="icon icon-message"></span></button>
		<h1 class="title">QQ用户绑定</h1>
		</header>
		<nav class="bar bar-tab">
		<a class="tab-item external active" href="index.cgi">
		<span class="icon icon-star"></span>
		<span class="tab-label">成绩查询</span>
		</a>
		</nav>
		<div class="content">
			<div class="content-block" style="overflow: hidden">
				<p id="login_query">
					%s
				</p>
				<canvas id="i_canvas" style="position:absolute;top:0;left:0;right:0;bottom:0"></canvas>
				<form action="OAuth2Assoc.cgi?openid=%s" method="post">
					<div class="list-block">
						<ul style="opacity: 0.75;z-index:9999">
							<li style="display:%s">
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-name"></i>
								</div>
								<div class="item-inner">
								<span class="icon icon-me"></span>
									<div class="item-title label">
										学号
									</div>
									<div class="item-input">
										<input id="i_xh" name="xh" type="text" placeholder="请输入学号" value="%s">
									</div>
								</div>
							</div>
							</li>
							<li style="display:%s">
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-password"></i>
								</div>
								<div class="item-inner">
								<span class="icon icon-edit"></span>
									<div class="item-title label">
										密码
									</div>
									<div class="item-input">
										<input name="mm" id="i_mm" type="password" placeholder="默认与学号相同" value="%s">
									</div>
								</div>
							</div>
							</li>
							<li>
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-name"></i>
								</div>
								<div class="item-inner">
								<span class="icon icon-picture"></span>
									<div class="item-title label">
										验证码
									</div>
									<div class="item-input">
										<input name="yzm" id="i_yzm" type="text" placeholder="输入下方验证码">
									</div>
								</div>
							</div>
							</li>
							<li class="align-top">
							<div class="item-content">
								<div class="item-media">
									<span id="login_hint">验证码图像</span><a onclick="get_captcha();" title="点击刷新" class="except"><img id="login_captcha" alt="" height="40" width="120" src="img/refresh.png"/></a>
								</div>
							</div>
							</li>
						</ul>
					</div>
					<div class="content-block">
						<div class="row">
							<div class="col-100">
								<input style="z-index:9999" id="i_submit" type="submit" value="绑定" class="button button-big button-fill" />
							</div>
						</div>
					</div>
				</div>
			</form>
		</div>
		<input id="about" style="display:none" value="<b>%s</b><br /><small>&copy; 2012-2017 iEdon Inside</br>编译于: %s %s<br />服务端环境: %s<br />QQ: 11595137<br />N加青年 · 提供</small>"/>
	</div>
</div>
<script type="text/javascript">
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
		},
		error: function(msg) {
			document.getElementById("login_captcha").src = "img/refresh.png";
			$.toast(msg);
		}
	})
}
get_captcha();
</script>