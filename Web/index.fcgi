<header class="demos-header">
	<h1 class="demos-title">
		%s
	</h1>
	<div class="weui-cells__title status">
		��&nbsp;<big>%d</big>&nbsp;λͬѧ������&nbsp;<big>%lld</big>&nbsp;�β�ѯ
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
						ѧ��
					</label>
				</div>
				<div class="weui-cell__bd">
					<input class="weui-input" id="i_xh" name="xh" type="text" placeholder="����ѧ��" value="%s" />
				</div>
				<div class="weui-cell__ft">
					<button type="button" class="weui-vcode-btn" onclick="autoreset();">
						�������
					</button>
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
						�Զ�����
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
					<input class="weui-input" name="yzm" id="i_yzm" type="text" placeholder="�����Ҳ���֤��" />
				</div>
				<div class="weui-cell__ft">
					<div class="weui-vcode-img"><img onclick="get_captcha();" id="login_captcha" alt="��֤��" src="img/refresh.png" width="60" height="20" /></div>
				</div>
			</div>
		</div>
		<label for="weuiAgree" class="weui-agree">
			<input id="weuiAgree" type="checkbox" class="weui-agree__checkbox" checked="checked">
			<span class="weui-agree__text">
				ͬ�����ǵķ�������
			</span>
		</label>
		<div class="weui-btn-area" style="display:%s">
			<input id="i_submit" type="submit" value="��¼" class="weui-btn weui-btn_primary col-50"/>
			<a title="QQ���ٵ�¼" class="weui-btn weui-btn_default col-50" href="OAuth2.fcgi">
				QQ���ٵ�¼
			</a>
		</div>
		<div class="quickquery" style="display:%s">
			<a class="weui-btn weui-btn_warn" href="QuickQuery.fcgi">
				���ܿ��ٲ�ѯ��� &gt;&gt;
			</a>
		</div>
		<div class="weui-btn-area" style="display:%s">
			<input id="i_submit" type="submit" value="����" class="weui-btn weui-btn_primary"/>
		</div>
	</form>
	<div class="weui-loadmore weui-loadmore_line">
		<span class="weui-loadmore__tips">
			����ʹ��ָ��
		</span>
	</div>
	<div class="weui-cells__title">
		<p>
			<b>1.</b>&nbsp;&nbsp;&nbsp;&nbsp;ʹ����Ľ���ϵͳ�ʺ������¼��(Ҳ����ѧУ������Ĳ�Ρ�ע���õ��ʺ�)
		</p>
		<p>
			<b>2.</b>&nbsp;&nbsp;&nbsp;&nbsp;�Զ���סѧ��������Ϣ(��Ϣ�Ѽ���)�����Զ�����ϴε�¼�ɹ����ʺ���Ϣ��
		</p>
		<p>
			<b>3.</b>&nbsp;&nbsp;&nbsp;&nbsp;����������룬����ϵУ����졣�������ʱ��ǧ��Ҫ��˼��~
		</p>
		<p>
			<b>4.</b>&nbsp;&nbsp;&nbsp;&nbsp;�Զ������Ȩƽ����(ԺУ�����ο�)��GPA(ƽ��ѧ�ּ��㣬����5.0)����������ѧ���ֲᡣ
		</p>
		<p>
			<b>5.</b>&nbsp;&nbsp;&nbsp;&nbsp;����ѧԺ���ṩ�����롢�ɶ��˲�ѯ�Ŀ��ٲ�ѯ���(���޲���ĩ�ɼ�)��ֻ��Ҫѧ�ž͹��ˡ�
		</p>
		<p>
			<b>6.</b>&nbsp;&nbsp;&nbsp;&nbsp;�������޷�ˢ����֤�룬���ε�����Ͻǵĵ�״ͼ�꣬Ȼ���ˢ�£�һ��Ҫ���ˢ�£����߸���������򿪣�΢��ҳ��Ļ���Ҳ�����ˡ�
		</p>
		<p>
			<b>7.</b>&nbsp;&nbsp;&nbsp;&nbsp;����������⣬����N�����깫�ں����԰巢����
		</p>
	</div>
	<div class="weui-loadmore weui-loadmore_line weui-loadmore_dot">
		<span class="weui-loadmore__tips">
		</span>
	</div>
</div>