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
				<form action="query.cgi" method="post">
				  <div class="list-block">
					<ul>
					  <!-- Text inputs -->
					  <li>
						<div class="item-content">
						  <div class="item-media"><i class="icon icon-form-name"></i></div>
						  <div class="item-inner">
							<div class="item-title label">学号</div>
							<div class="item-input">
							  <input name="xh" type="text" placeholder="请输入你的学号">
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
							  <input name="mm" type="password" placeholder="默认密码与学号相同" class="">
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
							  <input name="yzm" type="text" placeholder="请输入下方验证码">
							</div>
						  </div>
						</div>
					  </li>
					  <li class="align-top">
						<div class="item-content">
						  <div class="item-media">验证码图像：<img height="40" width="120"  src="%s" /></div>
						</div>
					  </li>
					</ul>
				  </div>
				  <div class="content-block">
					<div class="row">
					  <div class="col-100"><input type="submit" value="查询" class="button button-big button-fill button-success"></div>
					</div>
				  </div>
				</div>
				</form>
        </div>
       <!-- 其他的单个page内联页（如果有） -->
       <!-- <div class="page">...</div> -->
    </div>
    <script type='text/javascript' src='js/zepto.min.js' charset='utf-8'></script>
    <script type='text/javascript' src='js/sm.min.js' charset='utf-8'></script>
    <script type='text/javascript' src='js/sm-extend.min.js' charset='utf-8'></script>
    <!-- 默认必须要执行$.init(),实际业务里一般不会在HTML文档里执行，通常是在业务页面代码的最后执行 -->
    <!--<script>$.init()</script>-->
  </body>
</html>