# Media-pop-up



\[V1.1.0]

* You can now edit style sheet of all widgets in *stylesheet.qss* file.
* Added logging into file *logs/appLog.txt*



——————————————————————————————————————————————



Displays a pop-up window with animation whenever windows media player changes its data. Design is inspired by nvidia overlay pop-up thing that appears in the top right corner when you start a game.

The program extracts media properties from windows media player (you can see it when you press Win+A), displays the artist and the title in the window and chooses a small (<10 words) comment.
It looks for an object in commentlist.json that has the same artist and title and then picks the specified comment. Just take a look at the json file itself, you'll get what I mean.

Json looks like this:



*"Megadeth": {
"Symphony of destruction": "You try to take his BAAAAALLLSS; Mozart kicks in"
}, ...*



You specify artist name as a list and track title as an object inside the list. The value of that object will be you desired comment(s). You can specify several comments for a single track, and it will choose a random one of those, just separate comments with a semicolon.



