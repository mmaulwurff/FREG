-------------------------------------------------------------
freg text ("cursed") screen
===========================

Screen has two modes: wide and narrow.
Mode is set automatically depending on your terminal size.

Wide mode looks something like this:

    +---N----+ +---U----+
    |        | |        |
    W   @    E |        |
    |        | W        E
    |(1)     | |(2)     |
    +---S----+ +---D----+
    
    [health] a(3)z  [(4)]

    +---+  ---
    |(5)|  (6) (notifications)
    +---+  ---

1. is player view from above, similar to roguelike map.
2. is player first-person view.
At borders of 1 and 2 you see directions.
3. is mini-inventory. To view full inventory, type I or home.
4. is focused block information: durability and name.
5. is minimap. It shows area around you.
6. is action mode selection list.

Type `help visuals` for more information.
-------------------------------------------------------------
