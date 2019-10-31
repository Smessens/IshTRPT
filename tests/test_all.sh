# On lance le simulateur de lien avec
# un délais de 25 ms
# un taux d'erreur de 15%
# un taux de perte de 15%
# dans les deux sens
./../link_sim -p 64342 -P 64341 -d 25 -l 15 -e 15 &> link.log &
link_pid=$!

# On lance le receiver et capture sa sortie standard
valgrind --log-file=valgrind.txt ./../receiver -f "test_all_out.txt" :: 64341 &
receiver_pid=$!

cleanup()
{
    kill -9 $receiver_pid
    kill -9 $link_pid
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre le transfert
./../senderprof localhost 64342 < test_in.txt ;

sleep 15 # On attend 8 seconde que le receiver finisse

if kill -0 $receiver_pid &> /dev/null ; then
  echo "Le receiver ne s'est pas arreté à la fin du transfert!"
  kill -9 $receiver_pid
  err=1
fi

# On arrête le simulateur de lien
kill -9 $link_pid &> /dev/null
