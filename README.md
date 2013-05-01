VinaBit
=======

Network Programming Project

Bid theo thời gian, có min bid, max bid.<br/>
mỗi client có 2 kết nối đến server, một kết nối chính để client bid,login,logout,...<br/>
một kết nối phụ để cập nhật thông tin liên quan về mặt hàng: remaining time, ...<br/>

server có nhiều process con, phân làm 2 loại chính<br/>
loại 1: nhận kết nối từ phía client và xử lý<br/>
loại 2: chỉ có 1, là bộ đếm thời gian chính của server để quản lý thời gian bid sản phẩm<br/>

có sử dụng share memory để trao đổi thông tin giữa các process con.

HOW TO USE
==========

./configure<br/>
cd lib<br/>
make<br/>
cd ..<br/>
cd tcpcliserv<br/>
make<br/>

COMMAND FROM CLIENT
===================

AC_LOGIN name="username"<br/>
AC_LOGOUT<br/>
AC_BID val="money"  (for example money is "15.00")<br/>
