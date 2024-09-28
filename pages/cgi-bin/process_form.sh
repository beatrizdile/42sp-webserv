#!/bin/bash

# Define o tipo de conteúdo que será retornado (HTML)
echo "Content-type: text/html"
echo ""

# Função para decodificar dados de URL (como espaços, %20, etc.)
urldecode() {
    local url_encoded="${1//+/ }"
    printf '%b' "${url_encoded//%/\\x}"
}

# Lê os dados do formulário de acordo com o método de envio (POST ou GET)
if [ "$REQUEST_METHOD" = "POST" ]; then
    # Lê o conteúdo do stdin (corpo da requisição)
    read -n "$CONTENT_LENGTH" POST_DATA
    FORM_DATA=$POST_DATA
elif [ "$REQUEST_METHOD" = "GET" ]; then
    FORM_DATA=$QUERY_STRING
else
    echo "<h1>Método de requisição não suportado: $REQUEST_METHOD</h1>"
    exit 1
fi

# Separar e decodificar os dados do formulário
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

# Exibe os dados recebidos
echo "<html>"
echo "<head><title>Resposta do Formulário</title></head>"
echo "<body>"
echo "<h1>Dados Recebidos:</h1>"
echo "<p><strong>Nome:</strong> $nome</p>"
echo "<p><strong>Email:</strong> $email</p>"
echo "</body>"
echo "</html>"
