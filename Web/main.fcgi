<div class="i_page-navbar">
	%s
</div>
<div class="i_page-title">
	<div class="content">
		<div class="swiper-container" data-space-between='10' data-pagination='.swiper-pagination' data-autoplay='5000'>
		  <div class="swiper-wrapper">
			<div class="swiper-slide">
				<div class="i_header">
					<div class="i_user-photo-container">
						<img class="i_user-photo" src="/img/avatar.gif" width="84" height="84" />
					</div>
					<div class="i_user-info-container">
						<h1>%s</h1>
						<p><small>%s</small></p>
						<div class="info_tag">
							<a href="/changePassword.fcgi"><i class="fa fa-lock"></i>修改密码</a> · %s
						</div>
					</div>
				</div>
			</div>
			%s
		  </div>
		  <div class="swiper-pagination"></div>
		</div>
	</div>
</div>
<div class="content">
	<div class="weui-grids">
	  <a href="/TeachEval.fcgi" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/TeachEval.png" alt="">
		</div>
		<p class="weui-grid__label">
		  一键评教
		</p>
	  </a>
	  <a href="/query.fcgi" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/query.png" alt="">
		</div>
		<p class="weui-grid__label">
		  本学期成绩
		</p>
	  </a>
	  <a href="/query.fcgi?order=schedule" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/schedule.png" alt="">
		</div>
		<p class="weui-grid__label">
		  课程表
		</p>
	  </a>
		<a href="/query.fcgi?order=passed" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/passed.png" alt="">
		</div>
		<p class="weui-grid__label">
		  已过科目
		</p>
	  </a>
	  	<a href="/query.fcgi?order=byplan" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/plan.png" alt="">
		</div>
		<p class="weui-grid__label">
		  我的专业
		</p>
	  </a>
	    <a href="/query.fcgi?order=failed" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/failed.png" alt="">
		</div>
		<p class="weui-grid__label">
		  挂科查询
		</p>
	  </a>
	   <a href="/query.fcgi?order=tests" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/tests.png" alt="">
		</div>
		<p class="weui-grid__label">
		  月中补缓清考
		</p>
	  </a>
	  <a href="/Comments.html" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/Comments.png" alt="">
		</div>
		<p class="weui-grid__label">
		  交流反馈
		</p>
	  </a>
	  <a id="logout" onclick="logout();" href="javascript:void(0);" class="weui-grid js_grid">
		<div class="weui-grid__icon">
		  <img src="img/logout.png" alt="">
		</div>
		<p class="weui-grid__label">
		  退出登录
		</p>
	  </a>
	</div>
</div>