<!DOCTYPE html>
<html>
<head>
<meta charset="gb2312">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<title>iEdon URP查分助手</title>
<meta name="viewport" content="initial-scale=1, maximum-scale=1">
<link rel="shortcut icon" href="/favicon.ico">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black">
<link rel="stylesheet" href="css/sm.min.css">
<link rel="stylesheet" href="css/sm-extend.min.css">
</head>
<body>
<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-share"></span>&nbsp;&nbsp;<a onclick="return show_about();">关于</a></button>
		<button class="button button-link button-nav pull-right"><a href="Comments.html">留言</a>&nbsp;&nbsp;<span class="icon icon-message"></span></button>
		<h1 class="title">N加青年 广陵查分助手</h1>
		</header>
		<nav class="bar bar-tab">
		<a class="tab-item external active" href="index.cgi">
		<span class="icon icon-star"></span>
		<span class="tab-label">成绩查询</span>
		</a>
		</nav>
		<div class="content">
			<div class="content-block" style="overflow: hidden">
				<p>
					已成功完成 %ld 次查询操作
				</p>
				<p>
					输入你的教务系统账号来查询成绩 :)
				</p>
				<canvas id="i_canvas" style="position:absolute;top:0;left:0;right:0;bottom:0"></canvas>
				<form action="query.cgi" method="post">
					<div class="list-block">
						<ul style="opacity: 0.75;z-index:9999">
							<li>
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-name"></i>
								</div>
								<div class="item-inner">
									<div class="item-title label">
										学号
									</div>
									<div class="item-input">
										<input id="i_xh" name="xh" type="text" placeholder="请输入你的学号">
									</div>
								</div>
							</div>
							</li>
							<li>
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-password"></i>
								</div>
								<div class="item-inner">
									<div class="item-title label">
										密码
									</div>
									<div class="item-input">
										<input name="mm" id="i_mm" type="password" placeholder="默认密码与学号相同">
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
									<div class="item-title label">
										验证码
									</div>
									<div class="item-input">
										<input name="yzm" id="i_yzm" type="text" placeholder="请输入下方验证码">
									</div>
								</div>
							</div>
							</li>
							<li class="align-top">
							<div class="item-content">
								<div class="item-media">
									验证码图像：<a onclick="show_loading();" href="index.cgi" class="external" title="点击刷新"><img alt="" height="40" width="120" src="%s"/></a>
								</div>
							</div>
							</li>
						</ul>
					</div>
					<div class="content-block">
						<div class="row">
							<div class="col-100">
								<input style="z-index:9999" id="i_submit" type="submit" value="查询" class="button button-big button-fill button-success" />
							</div>
							<div class="col-100">
								<a style="z-index:9999;margin-top:10px" title="免密查询入口" onclick="show_loading();" class="button button-big button-fill external" href="QuickQuery.cgi">免密查询入口</a>
							</div>
						</div>
					</div>
				</div>
			</form>
		</div>
		<input id="about" style="display:none" value="<b>%s</b><br /><small>&copy; 2012-2017 iEdon Inside</br>编译于: %s %s<br />服务端环境: %s<br />QQ: 11595137<br />N加青年 · 提供</small>"/>
	</div>
</div>
<script type='text/javascript' src='js/zepto.min.js' charset='utf-8'></script>
<script type='text/javascript' src='js/sm.min.js' charset='utf-8'></script>
<script type='text/javascript' src='js/sm-extend.min.js' charset='utf-8'></script>
<script type='text/javascript' src='js/URPScoreHelper.js' charset='gb2312'></script>
<script>
var _mtac = {};
(function() {
    var mta = document.createElement("script");
    mta.src = "http://pingjs.qq.com/h5/stats.js?v2.0.2";
    mta.setAttribute("name", "MTAH5");
    mta.setAttribute("sid", "500426156");
    var s = document.getElementsByTagName("script")[0];
    s.parentNode.insertBefore(mta, s);
})();
</script>
</body>
</html>