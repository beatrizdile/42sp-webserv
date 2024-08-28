- [x] Retornar index quando ele existir em um diretório
- [] Tamanho dos arquivos esta errado no autoindex
- [] Revisar status de erro que estamos retornando
- [] Olhar para o método da requisição
- [x] Remover pastas e arquivos ocultos
- [x] Adicionar headers necessário nas respostas geradas com arquivos
  - Last-Modified
  - Etag
```c
ngx_sprintf(
  etag->value.data,
  "\"%xT-%xO\"",
  r->headers_out.last_modified_time,
  r->headers_out.content_length_n
);
```
