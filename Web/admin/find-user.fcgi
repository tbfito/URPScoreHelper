<!DOCTYPE html>
<html lang="zh-cn">
<head>
	<meta charset="utf-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
	<title>用户查询 - %s</title>
	<link rel="stylesheet" href="css/pintuer.css">
	<link rel="stylesheet" href="../css/font-awesome.min.css">
	<link rel="stylesheet" href="css/admin.css">
	<script src="../js/jquery.min.js"></script>
	<script src="js/pintuer.js"></script>
</head>
<body>
	<div class="panel admin-panel">
	  <div class="panel-head"><strong><i class="fa fa-user"></i> 用户查询</strong></div>
	  <div class="body-content">
		<form method="post" class="form-x" action="find-user.fcgi">
		  <div class="form-group">
			<div class="label">
			  <label for="sitename">请输入学生帐号：</label>
			</div>
			<div class="field">
				<input type="text" class="input" id="musr" name="STUDENT_ID" size="16" placeholder="请输入学生帐号" data-validate="required:请输入学生帐号" value="%s" autofocus /> 
			</div>
		  </div>
		  <div class="form-group">
			<div id="list_page">
			  <table width="100%" border="0" cellpadding="0" cellspacing="0" class="titleTop2">
				<tbody>
				  <tr>
					<td class="pageAlign">
					  <table cellpadding="0" width="100%" class="displayTag" cellspacing="1" border="0" id="user">
						<thead>
						  <tr>
								<th align="center" width="15%" class="sortable">ID</th>
								<th align="center" width="15%" class="sortable">密码</th>
								<th align="center" width="10%" class="sortable">姓名</th>
								<th align="center" width="30%" class="sortable">OpenID</th>
								<th align="center" width="10%" class="sortable">OAuth 用户名</th>
								<th align="center" width="20%" class="sortable">最后登录</th>
							</tr>
						</thead>
						<tbody>
						  <tr class="even" onmouseout="this.className='even';" onmouseover="this.className='evenfocus';">
								<td align="center">%s</td>
								<td align="center">%s</td>
								<td align="center">%s</td>
								<td align="center">%s</td>
								<td align="center">%s</td>
								<td align="center">%s</td>
							</tr>
						</tbody>
					  </table>
					</td>
				  </tr>
				</tbody>
			  </table>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label></label>
			</div>
			<div class="field">
			  <button class="button bg-main fa fa-check-square-o" type="submit"> 查询</button>   
			</div>
		  </div>      
		</form>
	  </div>
	</div>
</body>
</html>