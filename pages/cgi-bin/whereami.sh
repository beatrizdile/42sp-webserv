#!/bin/bash

echo "Content-type: text/html"
echo ""

cat << EOF

<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8" />
        <title>Diretorio de execução</title>
        <link rel="stylesheet" href="styles.css" />
        <link rel="icon" type="image/png" href="/resources/favicon.png">
    </head>
    <body>
        <div class="content">
        <h1>Diretorio de execução</h1>

        <div class="form-wrapper">
            <div class="label-wrapper">
                <p>O script está sendo executado no diretório: $(pwd)</p>
            </div>
        </div>
    </body>
</html>

EOF
