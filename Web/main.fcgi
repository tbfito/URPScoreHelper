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
							<a href="/changePassword.fcgi">修改密码</a> | <a style="display:%s" href="index.fcgi?act=requestAssoc">绑定QQ帐号</a>
							<div style="display:%s">
								<span style="color:rgb(0, 255, 90)">QQ已绑定</span> | <a href="javascript:void(0);" onclick="releaseAssoc('%s');">解除绑定</a>
							</div>
						</div>
					</div>
		</div>
	</div>
</div>
<div class="content">
	<div class="weui-grids">
	  <a href="TeachEval.fcgi" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/TeachEval.png" alt="">
		</div>
		<p class="weui-grid__label">
		  一键评教
		</p>
	  </a>
	  <a href="query.fcgi" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/query.png" alt="">
		</div>
		<p class="weui-grid__label">
		  本学期成绩
		</p>
	  </a>
	  <a href="query.fcgi?order=schedule" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/schedule.png" alt="">
		</div>
		<p class="weui-grid__label">
		  我的课程表
		</p>
	  </a>
		<a href="query.fcgi?order=passed" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/passed.png" alt="">
		</div>
		<p class="weui-grid__label">
		  已过科目
		</p>
	  </a>
	  	<a href="query.fcgi?order=byplan" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/plan.png" alt="">
		</div>
		<p class="weui-grid__label">
		  我的专业
		</p>
	  </a>
	    <a href="query.fcgi?order=failed" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/failed.png" alt="">
		</div>
		<p class="weui-grid__label">
		  挂科查询
		</p>
	  </a>
	   <a href="query.fcgi?order=tests" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/tests.png" alt="">
		</div>
		<p class="weui-grid__label">
		  月中补缓清考
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