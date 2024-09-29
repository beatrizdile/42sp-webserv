## verifica se o header cookie esta configurado e se tem a chave session_id com valor de um uuid
if [ -z "$HTTP_COOKIE" ] || [ -z "$(echo $HTTP_COOKIE | grep session_id)" ]; then
    echo "Content-type: text/html"
    echo ""
    echo "<h1>Cookie not found!!</h1>"
    exit 0
fi

session_id=$(echo $HTTP_COOKIE | grep -oP 'session_id=\K[^;]+')
if [ -z "$session_id" ]; then
    echo "Content-type: text/html"
    echo ""
    echo "<h1>Cookie not found!!</h1>"
    exit 0
fi

echo "Content-type: text/html"
echo ""
echo "<html>"
echo "<head><title>Just with login</title></head>"
echo "<body>"
echo "<h1>Just with login</h1>"
echo "<p>Session ID: $session_id</p>"
echo "</body>"
echo "</html>"

