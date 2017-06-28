<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-left"></span><a href="index.cgi">返回</a></button>
		<button class="button button-link button-nav pull-right"><a href="Comments.html">吐槽</a>&nbsp;&nbsp;<span class="icon icon-message"></span></button>
		<h1 class="title">广陵学院 · 免密成绩查询</h1>
		</header>
		<div class="content">
			<div class="content-block" style="overflow: hidden">
				<p>
					输入学号来查询成绩，暂时只支持广陵学院 :)
				</p>
				<form action="query.cgi?act=QuickQuery" method="post">
					<div class="list-block">
						<ul style="opacity: 0.75;z-index:9999">
							<li>
							<div class="item-content">
								<div class="item-media">
									<i class="icon icon-form-name"></i>
								</div>
								<div class="item-inner">
									<div class="item-title label">
										学号
									</div>
									<div class="item-input">
										<textarea id="i_xhs" name="xh" type="text" placeholder="多个学号换行输入(最多5个)"></textarea>
									</div>
								</div>
							</div>
							</li>
						</ul>
					</div>
					<div class="content-block">
						<div class="row">
							<div class="col-100">
								<input onclick="return check();" style="z-index:9999" id="i_submit" type="submit" value="免密查询" class="button button-big button-fill button-success" />
							</div>
							<div class="col-100">
								<a style="z-index:9999;margin-top:10px" title="&lt; 登录后综合查询" class="button button-big button-fill external" href="index.cgi">&lt; 登录后综合查询</a>
							</div>
						</div>
					</div>
				</div>
			</form>
			<center><p id="login_query">共&nbsp;&nbsp;<big>%ld</big>&nbsp;&nbsp;位同学进行了&nbsp;&nbsp;<big>%d</big>&nbsp;&nbsp;次查询</p></center>
		</div>
	</div>
</div>