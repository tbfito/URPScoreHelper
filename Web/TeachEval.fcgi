<div class="i_page-navbar">
	<a href="main.fcgi" class="return">&lt; �� ��</a>
	һ����ѧ����
</div>
<div class="content">
	<div id="i_total"><p>%s</p></div>
	<form action="TeachEval.fcgi?act=Evaluate" method="post" class="weui-cells weui-cells_form" style="display:%s">
		<div class="weui-cells weui-cells_form bigbox">
			<div class="weui-cell">
				<div class="weui-cell_bd">
					<textarea id="i_jxpg" class="weui-textarea" name="nr" type="text" placeholder="��ʦ��ô���࣬�������Ǻ����� :-)" onkeyup="getcharnum();" onchange="getcharnum();">��ʦ�����Ч���ڿ���ϸ�����û�Ծ��ÿ�ڿζ����ջ񣬸���ʦ����ޣ�</textarea>
					<div class="weui-textarea-counter"><span id="i_xhhs">0</span></div>
				</div>
			</div>
		</div>	
		<div class="weui-btn-area">
			<input id="i_submit" type="submit" value="һ������" class="weui-btn weui-btn_primary" />
		</div>
	</form>
	<br />
	%s
	<br />
</div>