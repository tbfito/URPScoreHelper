<div class="i_page-title">
	<div class="content">
		<div class="i_header">
					<div class="i_user-photo-container">
						<img class="i_user-photo" src="%s" width="84" height="84" />
					</div>
					<div class="i_user-info-container">
						<h1>%s</h1>
						<p><small>%s</small></p>
						<div style="padding-top:5px">
							<a href="/changePassword.cgi">修改密码</a> | <a style="display:%s" href="index.cgi?act=requestAssoc">绑定QQ帐号</a>
							<div style="display:%s">
								<span style="color:rgb(0, 255, 90)">&nbsp;QQ已绑定</span> | <a href="javascript:void(0);" onclick="releaseAssoc('%s');">解除绑定</a>
							</div>
						</div>
					</div>
		</div>
	</div>
</div>
<div class="content">
	<div class="weui-grids">
	  <a href="TeachEval.cgi" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/TeachEval.png" alt="">
		</div>
		<p class="weui-grid__label">
		  一键评教
		</p>
	  </a>
	  <a href="query.cgi" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/query.png" alt="">
		</div>
		<p class="weui-grid__label">
		  本学期成绩
		</p>
	  </a>
		<a href="query.cgi?order=tests" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/tests.png" alt="">
		</div>
		<p class="weui-grid__label">
		  月中补缓清考
		</p>
	  </a>
		<a href="query.cgi?order=passed" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/passed.png" alt="">
		</div>
		<p class="weui-grid__label">
		  已过科目
		</p>
	  </a>
		 <a href="query.cgi?order=byplan" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/plan.png" alt="">
		</div>
		<p class="weui-grid__label">
		  专业方案
		</p>
	  </a>
	   <a href="query.cgi?order=failed" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/failed.png" alt="">
		</div>
		<p class="weui-grid__label">
		  挂科查询
		</p>
	  </a>
		 <a onclick="logout();" href="javascript:void(0);" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/logout.png" alt="">
		</div>
		<p class="weui-grid__label">
		  退出登录
		</p>
	  </a>
	</div>
	<br />
</div>