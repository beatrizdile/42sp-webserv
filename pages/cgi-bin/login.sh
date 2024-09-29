echo "Content-type: text/html"
expires=$(LC_TIME=C date -u -d "+1 days" +"%a, %d %b %Y %H:%M:%S GMT")
echo "Set-Cookie: session_id=" $(uuidgen) "; Expires=$expires; Path=/; HttpOnly"
echo "Set-Cookie: user_name=42; Expires=$expires; Path=/"
echo ""

echo "<html>"
echo "<head><title>Login</title></head>"
echo "<body>"
echo "<h1>Login realizado!!</h1>"
echo "</form>"
echo "</body>"
echo "</html>"
