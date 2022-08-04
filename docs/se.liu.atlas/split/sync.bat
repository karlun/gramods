
start robocopy . "D:\Uniview\Settings\Playlist Items" MyReallyAwesomeMovie.fdv /timfix

:::: Primary node
start robocopy node0 \\ATLAS-00\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
start robocopy node0 \\ATLAS-00\UniviewVideos\MyReallyAwesomeMovie film.wav /timfix

:::: Stereo replica nodes
start robocopy node1 \\ATLAS-02\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
start robocopy node2 \\ATLAS-04\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
start robocopy node3 \\ATLAS-06\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
start robocopy node4 \\ATLAS-08\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
start robocopy node5 \\ATLAS-10\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
start robocopy node6 \\ATLAS-12\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix

:::: Unused single eye replica nodes
:: start robocopy node1 \\ATLAS-01\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
:: start robocopy node2 \\ATLAS-03\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
:: start robocopy node3 \\ATLAS-05\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
:: start robocopy node4 \\ATLAS-07\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
:: start robocopy node5 \\ATLAS-09\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
:: start robocopy node6 \\ATLAS-11\UniviewVideos\MyReallyAwesomeMovie film.mp4 /timfix
