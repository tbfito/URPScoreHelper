<header class="demos-header">
	<h1 class="demos-title">
		%s
	</h1>
	<div class="weui-cells__title status">
		��&nbsp;<big>%d</big>&nbsp;λͬѧ������&nbsp;<big>%ld</big>&nbsp;�β�ѯ
	</div>
</header>
<div class="content">
	<form action="query.fcgi?act=QuickQuery" method="post" class="weui-cells weui-cells_form">
		<div class="weui-cells__title">
			����ѧ������ѯ�ɼ�����֧�ֹ���ѧԺ :)
		</div>
		<div class="signbox">
			<div class="weui-cells weui-cells_form bigbox">
				  <div class="weui-cell">
					<div class="weui-cell__bd">
					  <textarea id="i_xh" name="xh" type="text" class="weui-textarea" placeholder="���ѧ�Ż�������(���5��)" rows="3" onchange="checkRows();" onkeyup="checkRows();"></textarea>
					  <div class="weui-textarea-counter"><span id="i_xhhs">0</span>/5</div>
					</div>
				  </div>
			</div>
		</div>
		<label for="weuiAgree" class="weui-agree">
			<input id="weuiAgree" type="checkbox" class="weui-agree__checkbox" checked="checked">
			<span class="weui-agree__text">
				ͬ�����ǵķ�������
			</span>
		</label>
		<div class="weui-btn-area">
			<input id="i_submit" type="submit" value="���ܲ�ѯ (��ĩ�ɼ�)" class="weui-btn weui-btn_primary" />
			<a title="&lt;&lt; ��¼���ۺϲ�ѯ" class="weui-btn weui-btn_default" href="/">
				&lt;&lt; ��¼���ۺϲ�ѯ
			</a>
		</div>
	</form>
	<br />
</div>