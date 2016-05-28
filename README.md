# SolderingStation
Schemetics and Code for a DIY weller soldering station
Find more infos at https://debugginglab.wordpress.com/2014/10/30/soldering-station/

pin 5 ist direkt an BL angeschlossen,
pin 7 auch aber verstärkt über einen Transistor,
pin 5 wurde als Eingang eingestellt und statt dessen wird pin 7 für die Hintergrundbeleuchtung verwendet

Die Farbübergänge waren nicht einstellbar und stellenweise zu dunkel. Sie sind jetzt immer gleich hell und die Temperaturabstände sind
einstellbar.

Die Anzeige war mir zu unruhig. Verschiedene Einstellmöglichkeiten für die Entprellung und Displayrefresh wurden hinzugefügt.

Maximale Leistung nur im standby reduziert, ansonsten 100%.

Textumbrüche ausgeschaltet wegen Anzeigeproblemen an manchen Displays.

PWM als Balkenanzeige

Nach 10 Minuten Nichtbenutzung (keine Änderung des SB-Status) Lötkolben ausschalten. Sobald der Lötkolben aus der Station aufgenommen oder in die Station abgelegt wird aktiviert sie sich wieder.
