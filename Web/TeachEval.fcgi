<div class="i_page-navbar">
	<a href="/main.fcgi" class="return"><i class="fa fa-arrow-left"></i>返回</a>
	<i class="fa fa-hand-o-right"></i>一键教学评估
</div>
<div class="i_page-padding"></div>
<div class="content">
	<div id="i_total"><p>%s</p></div>
	<form id="ajax_submit" data-ajax-submit="/TeachEval.fcgi?act=Evaluate" class="weui-cells weui-cells_form" style="display:%s">
		<div class="weui-cells weui-cells_form bigbox">
			<div class="weui-cell">
				<div class="weui-cell_bd">
					<textarea id="i_jxpg" class="weui-textarea" name="nr" type="text" placeholder="老师这么辛苦，给个五星好评吧 :)" onkeyup="getcharnum();" onchange="getcharnum();">老师认真高效，授课详细，课堂活跃，每节课都有收获，给老师点个赞！</textarea>
					<div class="weui-textarea-counter"><span id="i_xhhs">0</span></div>
				</div>
			</div>
		</div>	
		<div class="weui-btn-area">
			<button id="i_submit" type="button" class="weui-btn weui-btn_primary"><i class="fa fa-check-square-o"></i>一键评教</button>
		</div>
	</form>
	<br />
	%s
</div>