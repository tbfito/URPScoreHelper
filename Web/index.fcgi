<header class="demos-header">
	<h1 class="demos-title">%s</h1>
	<div class="weui-cells__title status">%s</div>
</header>
<div class="content">
	%s
	<form id="ajax_submit" data-ajax-submit="main.fcgi" class="weui-cells weui-cells_form">
		<div class="signbox">
			%s
			<div class="weui-cell weui-cell_vcode">
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
			<div class="weui-cell">
				<div class="weui-cell__hd">
					<label class="weui-label">
						密码
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" name="mm" id="i_mm" type="password" placeholder="输入密码" value="%s" />
				</div>
			</div>
			<div class="weui-cell weui-cell_vcode">
				<div class="weui-cell__hd">
					<label class="weui-label">
						验证码
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" name="yzm" id="i_yzm" type="text" placeholder="输入验证码" />
				</div>
				<div class="weui-cell__ft">
					<div class="weui-vcode-img"><img onclick="get_captcha();" id="login_captcha" alt="验证码" src="img/refresh.png" width="60" height="20" /></div>
				</div>
			</div>
		</div>
		<label for="weuiAgree" class="weui-agree">
			<input id="weuiAgree" type="checkbox" class="weui-agree__checkbox" checked="checked">
			<span class="weui-agree__text">
				同意服务条款
			</span>
		</label>
		<div class="weui-btn-area">
			<button id="i_submit" type="button" class="weui-btn weui-btn_primary%s">
				<i class="fa fa-check-square-o"></i>%s
			</button>
			%s
			%s
		</div>
	</form>
</div>