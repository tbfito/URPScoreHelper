<div class="i_page-navbar">
	<a href="main.fcgi" class="return"><i class="fa fa-arrow-left"></i>返回</a>
	<i class="fa fa-lock"></i>教务密码修改
</div>
<div class="i_page-padding"></div>
<div class="content">
	<div id="i_total"><p>修改的新密码请&nbsp;<b>务必记牢</b>，避免遗忘。</p></div>
	<form id="ajax_submit" data-ajax-submit="changePassword.fcgi" class="weui-cells weui-cells_form">
				<div class="weui-cell">
					<div class="weui-cell_hd">
						<label class="weui_label">新密码：&nbsp;&nbsp;</label>
					</div>
					<div class="weui-cell_bd">
						<input id="i_xhs" name="mm" tabindex="1" class="weui-input" type="password" placeholder="密码最长12位" />
					</div>
				</div>
				<div class="weui-cell">
					<div class="weui-cell_hd">
						<label class="weui_label">第二遍：&nbsp;&nbsp;</label>
					</div>
					<div class="weui-cell_bd">
						<input id="i_qr" tabindex="2" class="weui-input" type="password" placeholder="再次输入新密码" />
					</div>
				</div>
				<div class="weui-cell weui-cell_switch">
					<div class="weui-cell__bd">确定所输内容无误</div>
					<div class="weui-cell__ft">
					  <input id="i_chk" tabindex="3" class="weui-switch" type="checkbox">
					</div>
				</div>
		<div class="weui-btn-area">
			<button onclick="return change_password();" type="button" class="weui-btn weui-btn_primary"><i class="fa fa-check-square-o"></i>确认修改</button>
			<a href="main.fcgi" class="weui-btn weui-btn_default"><i class="fa fa-arrow-left"></i> 取消</a>
		</div>
	</form>
</div>
