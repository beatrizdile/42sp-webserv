#!/bin/bash

echo "Content-type: text/html"
echo ""

urldecode() {
    local url_encoded="${1//+/ }"
    printf '%b' "${url_encoded//%/\\x}"
}

if [ "$REQUEST_METHOD" = "POST" ]; then
    read -n "$CONTENT_LENGTH" POST_DATA
    FORM_DATA=$POST_DATA
    elif [ "$REQUEST_METHOD" = "GET" ]; then
    FORM_DATA=$QUERY_STRING
else
    echo "<h1>Método de requisição não suportado: $REQUEST_METHOD</h1>"
    exit 1
fi

for pair in $(echo "$FORM_DATA" | tr '&' '\n'); do
    key=$(echo "$pair" | cut -d '=' -f 1)
    value=$(echo "$pair" | cut -d '=' -f 2)
    decoded_value=$(urldecode "$value")
    
    case "$key" in
        nome)
            nome="$decoded_value"
        ;;
        email)
            email="$decoded_value"
        ;;
    esac
done

cat <<EOF

<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8" />
        <title>Resposta do Formulario</title>
        <link rel="stylesheet" href="styles.css" />
        <link rel="icon" type="image/png" href="/resources/favicon.png">
    </head>
    <body>
        <div class="content">
        <h1>Resposta do Formulario</h1>

        <div class="form-wrapper">
            <div class="label-wrapper">
                <p><strong>Nome:</strong> $nome</p>
                <p><strong>Email:</strong> $email</p>
            </div>
        </div>
    </body>
</html>

EOF
