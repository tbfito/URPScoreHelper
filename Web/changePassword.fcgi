<div class="i_page-navbar">
	<a href="main.fcgi" class="return">&lt; 返 回</a>
	教务密码修改
</div>
<div class="content">
	<div id="i_total"><p>修改密码要<b>慎重</b>呐，丢了可<b>找不回来</b>，除非找教务处 :)</p></div>
	<form action="changePassword.fcgi" method="post" class="weui-cells weui-cells_form">
				<div class="weui-cell">
					<div class="weui-cell_hd">
						<label class="weui_label">新密码：&nbsp;&nbsp;</label>
					</div>
					<div class="weui-cell_bd">
						<input id="i_xhs" name="mm" tabindex="1" class="weui-input" type="password" placeholder="最长12位" />
					</div>
				</div>
				<div class="weui-cell weui-cell_switch">
					<div class="weui-cell__bd">确认无误</div>
					<div class="weui-cell__ft">
					  <input id="i_chk" tabindex="2" class="weui-switch" type="checkbox">
					</div>
				</div>
		<div class="weui-btn-area">
			<input onclick="return check_password();" type="submit" value="确认修改" class="weui-btn weui-btn_primary" />
			<a href="main.fcgi" class="weui-btn weui-btn_default">&lt;&lt; 不整了，回去</a>
		</div>
	</form>
	<br />
</div>
