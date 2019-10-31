# On lance le simulateur de lien. Lien fiable
 #./../link_sim -p 64342 -P 64341 &> link.log &
 #link_pid=$!

# On lance le receiver
valgrind --log-file=valgrind.txt ./../receiver -o "test_basique_out.txt" :: 64342 &
receiver_pid=$!

cleanup()
{
    kill -9 $receiver_pid
    kill -9 $link_pid
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre le transfert
./../senderprof -f test_in.txt ::1 64342 ;

sleep 3 # On attend 8 seconde que le receiver finisse

if kill -0 $receiver_pid &> /dev/null ; then
  echo "Le receiver ne s'est pas arreté à la fin du transfert!"
  kill -9 $receiver_pid
  err=1
fi

# On arrête le simulateur de lien
 #kill -9 $link_pid &> /dev/null
