<div class="i_page-navbar">
	<a href="main.fcgi" class="return">&lt; 返 回</a>
	一键教学评估
</div>
<div class="content">
	<div id="i_total"><p>%s</p></div>
	<form action="TeachEval.fcgi?act=Evaluate" method="post" class="weui-cells weui-cells_form" style="display:%s">
		<div class="weui-cells weui-cells_form bigbox">
			<div class="weui-cell">
				<div class="weui-cell_bd">
					<textarea id="i_jxpg" class="weui-textarea" name="nr" type="text" placeholder="老师这么辛苦，给个五星好评吧 :-)" onkeyup="getcharnum();" onchange="getcharnum();">老师认真高效，授课详细，课堂活跃，每节课都有收获，给老师点个赞！</textarea>
					<div class="weui-textarea-counter"><span id="i_xhhs">0</span></div>
				</div>
			</div>
		</div>	
		<div class="weui-btn-area">
			<input id="i_submit" type="submit" value="一键评教" class="weui-btn weui-btn_primary" />
		</div>
	</form>
	<br />
	%s
	<br />
</div>