<header class="demos-header">
	<h1 class="demos-title">
		%s
	</h1>
	<div class="weui-cells__title status">
		欢迎使用 QQ登录功能
	</div>
</header>
<div class="content">
	<form id="ajax_submit" data-ajax-submit="/OAuth2Assoc.fcgi?openid=%s&proc=%s" class="weui-cells weui-cells_form">
		<div class="weui-cells__title">
			%s
		</div>
		<div class="signbox">
			<div class="weui-cell weui-cell_vcode">
				<div class="weui-cell__hd">
					<label class="weui-label">
						<i class="fa fa-user"></i>学号
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
			<div class="weui-cell weui-cell_vcode">
				<div class="weui-cell__hd">
					<label class="weui-label">
						<i class="fa fa-key"></i>密码
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
						<i class="fa fa-image"></i>验证码
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" name="yzm" id="i_yzm" type="text" placeholder="输入右侧验证码" />
				</div>
				<div class="weui-cell__ft">
					<div class="weui-vcode-img"><img onclick="get_captcha();" id="login_captcha" alt="验证码" src="/img/refresh.png" width="60" height="20" /></div>
				</div>
			</div>
		</div>
		<label for="weuiAgree" class="weui-agree">
			<input id="weuiAgree" type="checkbox" class="weui-agree__checkbox" checked="checked">
			<span class="weui-agree__text">
				同意我们的服务条款
			</span>
		</label>
		<div class="weui-btn-area">
			<button id="i_submit" type="button" class="weui-btn weui-btn_primary"><i class="fa fa-link"></i>绑定</button>
		</div>
	</form>
	<br />
</div>