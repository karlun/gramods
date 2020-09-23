# Cards

This is a set of Aruco board cards, each of 3x5 `4X4_100` markers.

Markers are 15 mm in size with 5 mm separation. The boards are created with the following command:

```{sh}
aruco_create_board -d=1 -h=5 -w=3 -l=6 -m=1 -s=2 -id=0  card_0.png
aruco_create_board -d=1 -h=5 -w=3 -l=6 -m=1 -s=2 -id=15 card_1.png
aruco_create_board -d=1 -h=5 -w=3 -l=6 -m=1 -s=2 -id=30 card_2.png
aruco_create_board -d=1 -h=5 -w=3 -l=6 -m=1 -s=2 -id=45 card_3.png
aruco_create_board -d=1 -h=5 -w=3 -l=6 -m=1 -s=2 -id=60 card_4.png
aruco_create_board -d=1 -h=5 -w=3 -l=6 -m=1 -s=2 -id=75 card_5.png
```
