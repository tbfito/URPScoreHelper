$(function () {
		$(document).on("click", "#i_submit", function(e) {
				document.oncontextmenu=new Function("event.returnValue=false;");
				   var r1 = document.getElementById("i_xh").value;
				   var r2 = document.getElementById("i_mm").value;
				   var r3 = document.getElementById("i_yzm").value;
				if(r1=="")
				{
					$.toast("(⊙o⊙) 学号还没输呢。");
					return false;
				}
				if(r2=="")
				{
					$.toast("(⊙o⊙) 密码还没输呢。");
					return false;
				}
				if(r3=="")
				{
					$.toast("(⊙o⊙) 验证码还没输呢。");
					return false;
				}
				show_querying();
				function show_querying() {
					$.showPreloader('O(∩_∩)O 正在查询');
					setTimeout(function () {
						$.hidePreloader();
					}, 10000);
				 }
		});
		
		$(document).on("pageInit", "", function(e, id, page) {
			//定义画布宽高和生成点的个数
			var WIDTH = window.innerWidth - 50, HEIGHT = window.innerHeight - 100, POINT = 20;
			
			var canvas = document.getElementById('i_canvas');
			canvas.width = WIDTH,
			canvas.height = HEIGHT;
			var context = canvas.getContext('2d');
			context.strokeStyle = 'rgba(0,0,0,0.2)',
			context.strokeWidth = 1,
			context.fillStyle = 'rgba(0,0,0,0.1)';
			var circleArr = [];

			//线条：开始xy坐标，结束xy坐标，线条透明度
			function Line (x, y, _x, _y, o) {
				this.beginX = x,
				this.beginY = y,
				this.closeX = _x,
				this.closeY = _y,
				this.o = o;
			}
			//点：圆心xy坐标，半径，每帧移动xy的距离
			function Circle (x, y, r, moveX, moveY) {
				this.x = x,
				this.y = y,
				this.r = r,
				this.moveX = moveX,
				this.moveY = moveY;
			}
			//生成max和min之间的随机数
			function num (max, _min) {
				var min = arguments[1] || 0;
				return Math.floor(Math.random()*(max-min+1)+min);
			}
			// 绘制原点
			function drawCricle (cxt, x, y, r, moveX, moveY) {
				var circle = new Circle(x, y, r, moveX, moveY)
				cxt.beginPath()
				cxt.arc(circle.x, circle.y, circle.r, 0, 2*Math.PI)
				cxt.closePath()
				cxt.fill();
				return circle;
			}
			//绘制线条
			function drawLine (cxt, x, y, _x, _y, o) {
				var line = new Line(x, y, _x, _y, o)
				cxt.beginPath()
				cxt.strokeStyle = 'rgba(0,0,0,'+ o +')'
				cxt.moveTo(line.beginX, line.beginY)
				cxt.lineTo(line.closeX, line.closeY)
				cxt.closePath()
				cxt.stroke();

			}
			//初始化生成原点
			function init () {
				circleArr = [];
				for (var i = 0; i < POINT; i++) {
					circleArr.push(drawCricle(context, num(WIDTH), num(HEIGHT), num(15, 2), num(10, -10)/40, num(10, -10)/40));
				}
				draw();
			}

			//每帧绘制
			function draw () {
				context.clearRect(0,0,canvas.width, canvas.height);
				for (var i = 0; i < POINT; i++) {
					drawCricle(context, circleArr[i].x, circleArr[i].y, circleArr[i].r);
				}
				for (var i = 0; i < POINT; i++) {
					for (var j = 0; j < POINT; j++) {
						if (i + j < POINT) {
							var A = Math.abs(circleArr[i+j].x - circleArr[i].x),
								B = Math.abs(circleArr[i+j].y - circleArr[i].y);
							var lineLength = Math.sqrt(A*A + B*B);
							var C = 1/lineLength*7-0.009;
							var lineOpacity = C > 0.03 ? 0.03 : C;
							if (lineOpacity > 0) {
								drawLine(context, circleArr[i].x, circleArr[i].y, circleArr[i+j].x, circleArr[i+j].y, lineOpacity);
							}
						}
					}
				}
			}
			//调用执行
				init();
				setInterval(function () {
					for (var i = 0; i < POINT; i++) {
						var cir = circleArr[i];
						cir.x += cir.moveX;
						cir.y += cir.moveY;
						if (cir.x > WIDTH) cir.x = 0;
						else if (cir.x < 0) cir.x = WIDTH;
						if (cir.y > HEIGHT) cir.y = 0;
						else if (cir.y < 0) cir.y = HEIGHT;
						
					}
					draw();
				}, 16);
		});
		
		$.init();
});