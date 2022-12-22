# rc-project

Project for Computer Networks class.

GAME file example:
GAME_099249.txt
```
word hint n_unique_chars t_count/g_count t_err/g_err/max_errors
banana hint 5 3/2 3/2/7
T a
G abacaxi
T b
T c
G lol
```

TODO:
- [DONE] games.c > quit()
- [DONE] games.c > rev()
- [DONE] games.c > archive_game()
- [DONE] games.c > save_score()
- [DONE] games.c > get_scoreboard()
- [DONE] games.c > get_hint_image()
- [DONE] games.c > get_state()
- [DONE] interface.c > process_tcp_message()
- [DONE] socket.c > [Escrever na socket a parte inicial, CMD STATUS FNAME FSIZE] - Usei o complete_write()
- [DONE] socket.c > complete_write_file_to_socket()
- [ ] GS.c > handling exit and abrupt exits (zombies and whatnot)
- [ ] GS.c > setsockopt
- [ ] player.c > setsockopt
- [ ] 1. Multithread UDP processing or 2. Implement request queueing
- [ ] Run scripts
- [ ] Wireshark demo game to see if everything's done
- [ ] Comment code + write README