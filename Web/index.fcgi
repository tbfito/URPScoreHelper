<header class="demos-header">
	<h1 class="demos-title">
		%s
	</h1>
	<div class="weui-cells__title status">
		共&nbsp;<big>%d</big>&nbsp;位同学进行了&nbsp;<big>%lld</big>&nbsp;次查询
	</div>
</header>
<div class="content">
	<form action="main.fcgi" method="post" class="weui-cells weui-cells_form">
		<div class="weui-cells__title">
			%s
		</div>
		<div class="signbox">
			<div class="weui-cell weui-cell_vcode" style="display:%s">
				<div class="weui-cell__hd">
					<label class="weui-label">
						学号
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" id="i_xh" name="xh" type="text" placeholder="输入学号" value="%s" />
				</div>
				<div class="weui-cell__ft">
					<button type="button" class="weui-vcode-btn" onclick="autoreset();">
						清空数据
					</button>
				</div>
			</div>
			<div class="weui-cell weui-cell_vcode" style="display:%s">
				<div class="weui-cell__hd">
					<label class="weui-label">
						密码
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" name="mm" id="i_mm" type="password" placeholder="默认相同" value="%s" />
				</div>
				<div class="weui-cell__ft">
					<button type="button" class="weui-vcode-btn" onclick="autofill();">
						自动补充
					</button>
				</div>
			</div>
			<div class="weui-cell weui-cell_vcode">
				<div class="weui-cell__hd">
					<label class="weui-label">
						验证码
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" name="yzm" id="i_yzm" type="text" placeholder="输入右侧验证码" />
				</div>
				<div class="weui-cell__ft">
					<div class="weui-vcode-img"><img onclick="get_captcha();" id="login_captcha" alt="验证码" src="img/refresh.png" width="60" height="20" /></div>
				</div>
			</div>
		</div>
		<label for="weuiAgree" class="weui-agree">
			<input id="weuiAgree" type="checkbox" class="weui-agree__checkbox" checked="checked">
			<span class="weui-agree__text">
				同意我们的服务条款
			</span>
		</label>
		<div class="weui-btn-area" style="display:%s">
			<input id="i_submit" type="submit" value="登录" class="weui-btn weui-btn_primary col-50"/>
			<a title="QQ快速登录" class="weui-btn weui-btn_default col-50" href="OAuth2.fcgi">
				QQ快速登录
			</a>
		</div>
		<div class="quickquery" style="display:%s">
			<a class="weui-btn weui-btn_warn" href="QuickQuery.fcgi">
				免密快速查询入口 &gt;&gt;
			</a>
		</div>
		<div class="weui-btn-area" style="display:%s">
			<input id="i_submit" type="submit" value="继续" class="weui-btn weui-btn_primary"/>
		</div>
	</form>
	<div class="weui-loadmore weui-loadmore_line">
		<span class="weui-loadmore__tips">
			懒人使用指南
		</span>
	</div>
	<div class="weui-cells__title">
		<p>
			<b>1.</b>&nbsp;&nbsp;&nbsp;&nbsp;使用你的教务系统帐号密码登录。(也就是学校发给你的查课、注册用的帐号)
		</p>
		<p>
			<b>2.</b>&nbsp;&nbsp;&nbsp;&nbsp;自动记住学号密码信息(信息已加密)，并自动填好上次登录成功的帐号信息。
		</p>
		<p>
			<b>3.</b>&nbsp;&nbsp;&nbsp;&nbsp;如果忘记密码，请联系校教务办。改密码的时候千万要三思哈~
		</p>
		<p>
			<b>4.</b>&nbsp;&nbsp;&nbsp;&nbsp;自动计算加权平均分(院校排名参考)，GPA(平均学分绩点，满分5.0)，依据来自学生手册。
		</p>
		<p>
			<b>5.</b>&nbsp;&nbsp;&nbsp;&nbsp;广陵学院版提供免密码、可多人查询的快速查询入口(仅限查期末成绩)，只需要学号就够了。
		</p>
		<p>
			<b>6.</b>&nbsp;&nbsp;&nbsp;&nbsp;若发现无法刷新验证码，请多次点击右上角的点状图标，然后点刷新！一定要多次刷新，或者更换浏览器打开，微信页面的缓存也是醉了。
		</p>
		<p>
			<b>7.</b>&nbsp;&nbsp;&nbsp;&nbsp;如果遇到问题，请在N加青年公众号留言板发布。
		</p>
	</div>
	<div class="weui-loadmore weui-loadmore_line weui-loadmore_dot">
		<span class="weui-loadmore__tips">
		</span>
	</div>
</div>