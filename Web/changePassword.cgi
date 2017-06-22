<script type="text/javascript">
	function check_password() {
		var r1 = document.getElementById("i_xhs").value;
		if(r1=="")
		{
			$.toast("(⊙o⊙) 总得输入些什么吧？");
			return false;
		}
		if(r1.length > 12)
		{
			$.toast("(⊙o⊙) 密码太长，破系统记不住");
			return false;
		}
		var patrn=/^(\w){6,12}$/;
		if (!patrn.exec(r1)){
			$.toast("只能输入6-12个字母、数字、下划线");
			return false;
		}
		if(!document.getElementById("i_chk").checked){
			$.toast("滑动右边开关来确认输入无误:-)");
			return false;
		}
		$.showPreloader('O(∩_∩)O 正在修改密码');
		setTimeout(function () {
			$.hidePreloader();
		}, 10000);
		return true;
	}
</script>
<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-left"></span><a href="main.cgi">返回</a></button>
		<button class="button button-link button-nav pull-right"><a href="Comments.html">吐槽</a>&nbsp;&nbsp;<span class="icon icon-message"></span></button>
		<h1 class="title">教务密码修改</h1>
		</header>
		<nav class="bar bar-tab">
		<a class="tab-item external active" href="main.cgi">
		<span class="icon icon-star"></span>
		<span class="tab-label">教务密码修改</span>
		</a>
		</nav>
		<div class="content">
			<div class="content-block" style="overflow: hidden">
				<p>
					<b>使用须知</b><small> | Xiu Xiu is more healthy.</small>
				</p>
				<p><small><b>为什么要修改密码？</b>众所周知，教务系统采用的默认密码等于学号，这给外界信息贩子带来了绝佳的机会。一旦穷举学号爬完学生数据，不法分子可以获得学生的一切信息（姓名、学号、证件照、专业、身份证、家庭住址等）进行敲诈、勒索。更有黑客会搜集这些信息录入社工库，那后果将会不堪设想。<b>对于广陵学院</b>，直接将教务系统暴露在公网，其数据安全性岌岌可危，另外，广陵学院会定期重置学生密码。所以，每年修改一次教务密码是必要的。这也是为什么小助手在广陵会启用HTTPS加密的原因之一。</small></p>
				<p><small><b>警告：</b>你将要修改密码，请仔细核对输入信息，修改后的密码请务必妥善保存。如果你忘记修改后的密码，我们将无能为力，届时只能求助学校教务部门。我们不承担任何风险责任。密码限制长度为12位，修改成功后，需要重新登录。</small></p>
				<canvas id="i_canvas" style="position:absolute;top:0;left:0;right:0;bottom:0"></canvas>
				<form action="changePassword.cgi" method="post">
					<div class="list-block">
						<ul style="opacity: 0.75;z-index:9999">
							<li>
								<div class="item-content">
									<div class="item-media">
										<i class="icon icon-form-name"></i>
									</div>
									<div class="item-inner">
										<div class="item-title label">
											新密码
										</div>
										<div class="item-input">
											<input name="mm" type="password" placeholder="最长12位" id="i_xhs">
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
										<div class="item-title label">
											确认输入无误
										</div>
										<label class="label-switch">
												<input type="checkbox" id="i_chk">
												<div class="checkbox"></div>
										</label>
									</div>
								</div>
							</li>
						</ul>
					</div>
					<div class="content-block">
						<div class="row">
							<div class="col-100">
								<input onclick="return check_password();" style="z-index:9999" type="submit" value="确认修改" class="button button-big button-fill button-success" />
							</div>
							<div class="col-100">
								<a style="z-index:9999;margin-top:10px" title="&lt; 不整了，回去" class="button button-big button-fill external" href="main.cgi">&lt; 不整了，回去</a>
							</div>
						</div>
					</div>
				</div>
			</form>
			<center><p id="login_query">共&nbsp;&nbsp;<big>%ld</big>&nbsp;&nbsp;位同学进行了&nbsp;&nbsp;<big>%d</big>&nbsp;&nbsp;次查询</p></center>
		</div>
	</div>
</div>