<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-share"></span>&nbsp;&nbsp;<a class="except" onclick="return show_about();">����</a></button>
		<button class="button button-link button-nav pull-right"><a href="Comments.html">�²�</a>&nbsp;&nbsp;<span class="icon icon-message"></span></button>
		<h1 class="title">QQ�û���</h1>
		</header>
		<div class="content">
			<div class="content-block" style="overflow: hidden">
				<p id="login_query">
					%s
				</p>
				<form action="OAuth2Assoc.cgi?openid=%s" method="post">
					<div class="list-block">
						<ul style="opacity: 0.75;z-index:9999">
							<li style="display:%s">
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-name"></i>
								</div>
								<div class="item-inner">
								<span class="icon icon-me"></span>
									<div class="item-title label">
										ѧ��
									</div>
									<div class="item-input">
										<input id="i_xh" name="xh" type="text" placeholder="������ѧ��" value="%s">
									</div>
								</div>
							</div>
							</li>
							<li style="display:%s">
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-password"></i>
								</div>
								<div class="item-inner">
								<span class="icon icon-edit"></span>
									<div class="item-title label">
										����
									</div>
									<div class="item-input">
										<input name="mm" id="i_mm" type="password" placeholder="Ĭ����ѧ����ͬ" value="%s">
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
										��֤��
									</div>
									<div class="item-input">
										<input name="yzm" id="i_yzm" type="text" placeholder="�����·���֤��">
									</div>
								</div>
							</div>
							</li>
							<li class="align-top">
							<div class="item-content">
								<div class="item-media">
									<span id="login_hint">��֤��ͼ��</span><a onclick="get_captcha();" title="���ˢ��" class="except"><img id="login_captcha" alt="" height="40" width="120" src="img/refresh.png"/></a>
								</div>
							</div>
							</li>
						</ul>
					</div>
					<div class="content-block">
						<div class="row">
							<div class="col-100">
								<input style="z-index:9999" id="i_submit" type="submit" value="��" class="button button-big button-fill" />
							</div>
						</div>
					</div>
				</div>
			</form>
		</div>
		<input id="about" style="display:none" value="<b>%s</b><br /><small>&copy; 2012-2017 iEdon Inside</br>������: %s %s<br />����˻���: %s<br />QQ: 11595137<br />N������ �� �ṩ</small>"/>
	</div>
</div>
<script type="text/javascript">
get_captcha();
</script>