# Projeto RC

Projeto realizado pelo grupo 81:
- Beatriz Matias, nº 95538
- João Sereno, nº 99249

# Estrutura de Ficheiros

```tree
81/
├── bin/
│   ├── GS.o
│   ├── interface.o
│   └── player.o
├── GAMES/
│   ├── 099249/
│   │   ├── YYYYMMDD_HHMMSS_W.txt
│   │   ├── YYYYMMDD_HHMMSS_F.txt
│   │   └── YYYYMMDD_HHMMSS_Q.txt
│   ├── 095538/
│   │   └── YYYYMMDD_HHMMSS_W.txt
│   └── GAME_099249.txt
├── SCORES/
│   ├── 100_099249_DDMMYYYY_HHMMSS.txt
│   └── 001_099249_DDMMYYYY_HHMMSS.txt
├── src/
│   ├── data/
│   │   ├── images/
│   │   │   └── ...
│   │   └── words/
│   │       └── word_eng.txt
│   ├── common.h
│   ├── games.c
│   ├── games.h
│   ├── GS.c
│   ├── interface.c
│   ├── interface.h
│   ├── player.c
│   ├── session.c
│   ├── session.h
│   ├── socket.c
│   └── socket.h
├── GS
├── Makefile
├── player
└── README.md
```

A diretoria que inclui as images (src/data/images/) está vazia. Por favor, coloque as imagens nessa diretoria, bem como ficheiros de palavras adicionais na diretoria (src/data/words/).

# Notas

1. É de salientar que fizemos uma alteração ao formato de como são guardados os ficheiros correspondentes ao jogo pelo servidor. Estes, na primeira linha, incluem várias informações que são atualizadas no decorrer do jogo. A primeira linha terá a seguinte estrutra `<word> <hint> <n_unique_chars> <t_count>/<g_count> <t_err>/<g_err>/<max_errors>`.
Segue-se um exemplo:
```
abracadabra witch.jpg 5 2/3 0/2/9
T a
G abacaxi
T b
G teste
G abracadabra
```

2. Para desativar o sistema de reenvio de mensagens não recebidas pelo jogador terá de adicionar a flag '-t' na execução do aplicativo.
Por exemplo:

```
./player -n tejo.ulisboa.tecnico.pt -p 58011 -t
```

> Esta flag deverá ser ativada quando se pretende utilizar o comando "REV" pois o servidor não responde ao mesmo.

3. Implementámos uma estrutura de listas ligadas circulares para guardar a sequência de palavras que podem ser escolhidas sequencialmente. Por escassez de tempo, não implementámos um sistema de aleatoriedade que seria ativado pela flag '-r' ao correr o servidor, mas encontra-se comentado no método de escolha de palavra como se deveria proceder.

