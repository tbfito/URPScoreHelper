<header class="demos-header">
	<h1 class="demos-title">
		%s
	</h1>
	<div class="weui-cells__title status">
		QQ登录功能
	</div>
</header>
<div class="content">
	<form action="OAuth2Assoc.cgi?openid=%s" method="post" class="weui-cells weui-cells_form">
		<div class="weui-cells__title">
			%s
		</div>
		<div class="signbox">
			<div class="weui-cell" style="display:%s">
				<div class="weui-cell__hd">
					<label class="weui-label">
						学号
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" id="i_xh" name="xh" type="text" placeholder="请输入学号" value="%s" />
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
						自动填写密码
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
					<input class="weui-input" name="yzm" id="i_yzm" type="text" placeholder="请输入验证码" />
				</div>
				<div class="weui-cell__ft">
					<img onclick="get_captcha();" class="weui-vcode-img" id="login_captcha" alt="验证码" src="img/refresh.png" width="120" height="44" />
				</div>
			</div>
		</div>
		<div class="weui-btn-area">
			<input id="i_submit" type="submit" value="绑定" class="weui-btn weui-btn_primary"/>
		</div>
	</form>
	<br />
</div>