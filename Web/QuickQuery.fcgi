<header class="demos-header">
	<h1 class="demos-title">
		%s
	</h1>
	<div class="weui-cells__title status">
		共&nbsp;<big>%d</big>&nbsp;位同学进行了&nbsp;<big>%d</big>&nbsp;次查询
	</div>
</header>
<div class="content">
	<form action="query.fcgi?act=QuickQuery" method="post" class="weui-cells weui-cells_form">
		<div class="weui-cells__title">
			输入学号来查询成绩，仅支持部分院校 :)
		</div>
		<div class="signbox">
			<div class="weui-cells weui-cells_form bigbox">
				  <div class="weui-cell">
					<div class="weui-cell__bd">
					  <textarea id="i_xh" name="xh" type="text" class="weui-textarea" placeholder="多个学号换行输入(最多5个)" rows="3" onchange="checkRows();" onkeyup="checkRows();"></textarea>
					  <div class="weui-textarea-counter"><span id="i_xhhs">0</span>/5</div>
					</div>
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
			<button id="i_submit" type="submit" class="weui-btn weui-btn_primary"><i class="fa fa-search"></i>快速查询 (期末成绩)</button>
			<a title="&lt;&lt; 登录后综合查询" class="weui-btn weui-btn_default" href="/">
				<i class="fa fa-arrow-left"></i>登录后综合查询
			</a>
		</div>
	</form>
	<br />
</div>