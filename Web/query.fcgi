<!DOCTYPE html>
<html>
<head>
<meta charset="gb2312">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<title>%s的成绩 - 广陵学院教务系统</title>
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
		<button class="button button-link button-nav pull-left">
		<span class="icon icon-left"></span><a href="index.fcgi">返回</a></button>
		<h1 class="title">查询成绩 · %s</h1>
		</header>
		<nav class="bar bar-tab">
		<a class="tab-item external active" href="index.fcgi">
		<span class="icon icon-star"></span>
		<span class="tab-label">成绩查询</span>
		</a>
		</nav>
		<canvas id="i_canvas" style="position:absolute;top:0;left:0;right:0;bottom:0"></canvas>
		<div class="content" style="opacity: 0.75;z-index:9999">
			  %s
		</div>
	</div>
</div>
<script type='text/javascript' src='js/zepto.min.js' charset='utf-8'></script>
<script type='text/javascript' src='js/sm.min.js' charset='utf-8'></script>
<script type='text/javascript' src='js/sm-extend.min.js' charset='utf-8'></script>
<script type='text/javascript' src='js/GuanglingScoreHelper.js' charset='gb2312'></script>
</body>
</html>