<header class="demos-header">
	<h1 class="demos-title">
		%s
	</h1>
	<div class="weui-cells__title status">
		QQ��¼����
	</div>
</header>
<div class="content">
	<form action="OAuth2Assoc.fcgi?openid=%s" method="post" class="weui-cells weui-cells_form">
		<div class="weui-cells__title">
			%s
		</div>
		<div class="signbox">
			<div class="weui-cell" style="display:%s">
				<div class="weui-cell__hd">
					<label class="weui-label">
						ѧ��
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" id="i_xh" name="xh" type="text" placeholder="����ѧ��" value="%s" />
				</div>
			</div>
			<div class="weui-cell weui-cell_vcode" style="display:%s">
				<div class="weui-cell__hd">
					<label class="weui-label">
						����
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" name="mm" id="i_mm" type="password" placeholder="Ĭ����ͬ" value="%s" />
				</div>
				<div class="weui-cell__ft">
					<button type="button" class="weui-vcode-btn" onclick="autofill();">
						�Զ���д����
					</button>
				</div>
			</div>
			<div class="weui-cell weui-cell_vcode">
				<div class="weui-cell__hd">
					<label class="weui-label">
						��֤��
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" name="yzm" id="i_yzm" type="text" placeholder="������֤��" />
				</div>
				<div class="weui-cell__ft">
					<img onclick="get_captcha();" class="weui-vcode-img" id="login_captcha" alt="��֤��" src="img/refresh.png" width="124" height="44" />
				</div>
			</div>
		</div>
		<div class="weui-btn-area">
			<input id="i_submit" type="submit" value="��" class="weui-btn weui-btn_primary"/>
		</div>
	</form>
	<br />
</div>