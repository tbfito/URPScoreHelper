<div class="page-group">
	<div class="page">
		<header class="bar bar-nav">
		<button class="button button-link button-nav pull-left"><span class="icon icon-share"></span>&nbsp;&nbsp;<a class="except" onclick="return show_about();">����</a></button>
		<h1 class="title">Ψ��С����</h1>
		</header>
		<div class="content">
			<div class="content-block" style="overflow: hidden">
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
							  <div class="item-subtitle"><small>������Ϣ:</small></div>
							  <div class="item-subtitle">%s</div>
							  <div class="item-subtitle">%s | <a href="/changePassword.cgi">�޸�����</a></div>
							  <div class="item-subtitle" style="display:%s"><a href="index.cgi?act=requestAssoc">��QQ�ʺ�</a></div>
							  <div class="item-subtitle" style="display:%s"><span style="color:#4cd964" class="icon icon-emoji">&nbsp;QQ�Ѱ�</span><a href="javascript:void(0);" onclick="window.confirm('��ȷ��Ҫ�����ѧ����QQ�ʻ��Ĺ�����')?this.href='OAuth2Assoc.cgi?release=%s':this.href='javascript:window.location.reload();';">�����</a></div>
							</div>
						  </li>
						  <li class="item-content">
						  Ҫ��ʲô�أ�
						  </li>
						</ul>
						<ul>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px;" class="button button-big button-success button-fill" href="query.cgi">��ѧ�ڳɼ�</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px;background-color: #ec4dbc !important" class="button button-big button-warning button-fill" href="TeachEval.cgi">һ������</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-warning button-fill" href="query.cgi?order=tests">���в����忼</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-fill" href="query.cgi?order=passed">�ѹ���Ŀ</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-dark button-fill" href="query.cgi?order=byplan">רҵ����</a></li>
							<li style="width:50%%;float:left;padding:0 6px 4px 6px"><a style="z-index:9999;margin-top:10px" class="button button-big button-danger button-fill" href="query.cgi?order=failed">�ҿƲ�ѯ</a></li>
						</ul>
					  </div>
					</div>
					<div class="card-footer">
					  <a href="index.cgi?act=logout" class="link">�˳���¼</a>
					</div>
		</div>
		<input id="about" style="display:none" value="<b>%s</b><br /><small>&copy; 2012-2017 iEdon Inside</br>������: %s %s<br />����˻���: %s<br />QQ: 11595137<br />N������ �� �ṩ</small>"/>
	</div>
</div>