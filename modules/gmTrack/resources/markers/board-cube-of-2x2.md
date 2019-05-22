# Board cube

This is an Aruco board that forms a 4 cm cube where each side has a 2x2 board of `4X4_50` markers.

Markers are 12 mm in size with 4 mm separation and 6 mm margin to the border of the board and to the edge of the cube. The boards for the sides were created with the following command:

```{sh}
aruco_create_board -d=0 -h=2 -w=2 -l=6 -m=3 -s=2 -id=0 board-2x2_4X4_50-0.png
aruco_create_board -d=0 -h=2 -w=2 -l=6 -m=3 -s=2 -id=4 board-2x2_4X4_50-4.png
aruco_create_board -d=0 -h=2 -w=2 -l=6 -m=3 -s=2 -id=8 board-2x2_4X4_50-8.png
aruco_create_board -d=0 -h=2 -w=2 -l=6 -m=3 -s=2 -id=12 board-2x2_4X4_50-12.png
aruco_create_board -d=0 -h=2 -w=2 -l=6 -m=3 -s=2 -id=16 board-2x2_4X4_50-16.png
aruco_create_board -d=0 -h=2 -w=2 -l=6 -m=3 -s=2 -id=20 board-2x2_4X4_50-20.png
```
