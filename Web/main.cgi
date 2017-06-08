<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-share"></span>&nbsp;&nbsp;<a class="except" onclick="return show_about();">关于</a></button>
		<button class="button button-link button-nav pull-right"><a href="Comments.html">交流</a>&nbsp;&nbsp;<span class="icon icon-message"></span></button>
		<h1 class="title">N加青年 广陵查分助手</h1>
		</header>
		<nav class="bar bar-tab">
		<a class="tab-item active" href="index.cgi">
		<span class="icon icon-star"></span>
		<span class="tab-label">成绩查询</span>
		</a>
		</nav>
		<div class="content">
			<div class="content-block" style="overflow: hidden">
				<canvas id="i_canvas" style="position:absolute;top:0;left:0;right:0;bottom:0"></canvas>
				 <div class="card" style="opacity: 0.85;z-index:9999">
					<div class="card-header">%s</div>
					<div class="card-content">
					  <div class="list-block media-list">
						<ul>
						  <li class="item-content">
							<div class="item-media">
							  <img id="main_photo" src="%s" width="70" height="84" />
							</div>
							<div class="item-inner">
							  <div class="item-subtitle"><small>个人信息:</small></div>
							  <div class="item-subtitle">%s</div>
							  <div class="item-subtitle">%s | <a href="/changePassword.cgi">修改密码</a></div>
							  <div class="item-subtitle" style="display:%s"><a href="index.cgi?act=requestAssoc">绑定QQ帐号</a></div>
							  <div class="item-subtitle" style="display:%s"><span style="color:#4cd964" class="icon icon-emoji">&nbsp;QQ已绑定</span><a href="javascript:void(0);" onclick="window.confirm('你确定要解除该学号与QQ帐户的关联吗？')?this.href='OAuth2Assoc.cgi?release=%s':this.href='javascript:window.location.reload();';">解除绑定</a></div>
							</div>
						  </li>
						  <li class="item-content">
						  要做什么呢？
						  </li>
						</ul>
						<ul>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px;" class="button button-big button-success button-fill" href="query.cgi">期末排名GPA</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px;background-color: #ec4dbc !important" class="button button-big button-warning button-fill" href="TeachEval.cgi">一键评教</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-warning button-fill" href="query.cgi?order=tests">月中补缓清考</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-fill" href="query.cgi?order=passed">已过科目(总)</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-dark button-fill" href="query.cgi?order=byplan">专业方案</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-danger button-fill" href="query.cgi?order=failed">挂科查询</a></li>
						</ul>
					  </div>
					</div>
					<div class="card-footer">
					  <a href="index.cgi?act=logout" class="link">退出登录</a>
					</div>
		</div>
		<input id="about" style="display:none" value="<b>%s</b><br /><small>&copy; 2012-2017 iEdon Inside</br>编译于: %s %s<br />服务端环境: %s<br />QQ: 11595137<br />N加青年 · 提供</small>"/>
	</div>
</div>