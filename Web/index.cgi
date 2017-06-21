<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-share"></span>&nbsp;&nbsp;<a class="except" onclick="return show_about();">关于</a></button>
		<button class="button button-link button-nav pull-right"><a href="Comments.html">吐槽</a>&nbsp;&nbsp;<span class="icon icon-message"></span></button>
		<h1 class="title">唯扬小助手</h1>
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
					共&nbsp;&nbsp;<big>%ld</big>&nbsp;&nbsp;位同学进行了&nbsp;&nbsp;<big>%d</big>&nbsp;&nbsp;次查询
				</p>
				%s
				<canvas id="i_canvas" style="position:absolute;top:0;left:0;right:0;bottom:0"></canvas>
				<form action="main.cgi" method="post">
					<div class="list-block">
						<ul style="opacity: 0.75;z-index:9999;display:%s">
							<li>
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
							<li>
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
						</ul>
						<ul style="opacity: 0.75;z-index:9999">
							<li>
								<div class="item-content">
									<div class="item-media"></div>
									<div class="item-inner">
										<div class="item-title label">
											验证码图像
										</div>
										<div class="item-input">
											<a onclick="get_captcha();" title="点击刷新" class="except"><img id="login_captcha" alt="" height="40" width="120" src="img/refresh.png"/></a>
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
											<input name="yzm" id="i_yzm" type="text" placeholder="输入上方验证码">
										</div>
									</div>
								</div>
							</li>
						</ul>
					</div>
					<div class="content-block">
						<div class="row" style="display:%s">
							<div class="col-50">
								<input style="z-index:9999" id="i_submit" type="submit" value="登录" class="button button-big button-fill button-success" />
							</div>
							<div class="col-50">
								<a style="z-index:9999;" title="QQ快速登录" class="button button-big button-fill button-success external" href="OAuth2.cgi">QQ快速登录</a>
							</div>
							<!--<div class="col-100">
								<a style="z-index:9999;margin-top:10px" title="免密查询入口" class="button button-big button-fill external" href="QuickQuery.cgi">免密查询入口</a>
							</div>-->
						</div>
						<div class="row" style="display:%s">
							<div class="col-100">
								<input style="z-index:9999" id="i_submit" type="submit" value="继续" class="button button-big button-fill" />
							</div>
						</div>
					</div>
				</form>
				<center><p>苏ICP备15008822号-1</p></center>
		</div>
		<input id="about" style="display:none" value="<b>%s</b><br /><small>&copy; 2012-2017 iEdon Inside</br>编译于: %s %s<br />服务端环境: %s<br />QQ: 11595137<br />N加青年 · 提供</small>"/>
	</div>
</div>
<script type="text/javascript">
get_captcha();
</script>