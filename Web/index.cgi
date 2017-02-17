<!DOCTYPE html>
<html>
  <head>
    <meta charset="gb2312">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <title>广陵学院教务系统 - 成绩查询</title>
    <meta name="viewport" content="initial-scale=1, maximum-scale=1">
    <link rel="shortcut icon" href="/favicon.ico">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black">
    <link rel="stylesheet" href="css/sm.min.css">
    <link rel="stylesheet" href="css/sm-extend.min.css">
  </head>
 <body>
<!-- page集合的容器，里面放多个平行的.page，其他.page作为内联页面由路由控制展示 -->
    <div class="page-group">
        <!-- 单个page ,第一个.page默认被展示-->
        <div class="page">
            <!-- 标题栏 -->
            <header class="bar bar-nav">
                <h1 class="title">广陵学院 · 学生成绩查询</h1>
            </header>
            <!-- 工具栏 -->
            <nav class="bar bar-tab">
                <a class="tab-item external active" href="index.cgi">
                    <span class="icon icon-star"></span>
                    <span class="tab-label">成绩查询</span>
                </a>
            </nav>
            <!-- 这里是页面内容区 -->
				<div class="content">
				<div class="content-block" style="overflow: hidden">
				<p>我已为小伙伴们提供了 %ld 次查询操作~</p>
				<p>输入学生信息来查询成绩 :)</p>
				<canvas id="i_canvas" style="position:absolute;top:0;left:0;right:0;bottom:0"></canvas>
				<form action="query.cgi" method="post">
				  <div class="list-block">
					<ul style="opacity: 0.75;z-index:9999">
					  <!-- Text inputs -->
					  <li>
						<div class="item-content">
						  <div class="item-media"><i class="icon icon-form-name"></i></div>
						  <div class="item-inner">
							<div class="item-title label">学号</div>
							<div class="item-input">
							  <input id="i_xh" name="xh" type="text" placeholder="请输入你的学号">
							</div>
						  </div>
						</div>
					  </li>
					  <li>
						<div class="item-content">
						  <div class="item-media"><i class="icon icon-form-password"></i></div>
						  <div class="item-inner">
							<div class="item-title label">密码</div>
							<div class="item-input">
							  <input name="mm" id="i_mm" type="password" placeholder="默认密码与学号相同">
							</div>
						  </div>
						</div>
					  </li>
					  <li>
						<div class="item-content">
						  <div class="item-media"><i class="icon icon-form-name"></i></div>
						  <div class="item-inner">
							<div class="item-title label">验证码</div>
							<div class="item-input">
							  <input name="yzm" id="i_yzm" type="text" placeholder="请输入下方验证码">
							</div>
						  </div>
						</div>
					  </li>
					  <li class="align-top">
						<div class="item-content">
						  <div class="item-media">验证码图像：<a href="index.cgi" title="点击刷新"><img alt="" height="40" width="120" src="%s" /></a></div>
						</div>
					  </li>
					</ul>
				  </div>
				  <div class="content-block">
					<div class="row">
					  <div class="col-100"><input style="z-index:9999" id="i_submit" type="submit" value="查询" class="button button-big button-fill button-success"></div>
					</div>
				  </div>
				</div>
				</form>
				</div>
        </div>
       <!-- 其他的单个page内联页（如果有） -->
       <!-- <div class="page">...</div> -->
    </div>
	<script type='text/javascript' src='js/zepto.min.js' charset='utf-8'></script>
    <script type='text/javascript' src='js/sm.min.js' charset='utf-8'></script>
    <script type='text/javascript' src='js/sm-extend.min.js' charset='utf-8'></script>
	<script type='text/javascript' src='js/GuanglingScoreHelper.js' charset='gb2312'></script>
  </body>
</html>