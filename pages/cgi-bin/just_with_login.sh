echo "Content-type: text/html"
echo ""

title="Pagina logada"
session_id=$(echo $HTTP_COOKIE | grep -oP 'session_id=\K[^;]+')
text="Session ID: $session_id"
if [ -z "$session_id" ]; then
    title="Não autenticado"
    text="Você não está autenticado"
fi

cat <<EOF

<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8" />
        <title>Pagina com autenticação</title>
        <link rel="stylesheet" href="styles.css" />
        <link rel="icon" type="image/png" href="/resources/favicon.png">
    </head>
    <body>
        <div class="content">
        <h1>$title</h1>

        <div class="form-wrapper">
            <div class="label-wrapper">
                <p>$text</p>
            </div>
        </div>
    </body>
</html>

EOF
