# About:
library footprint SHIELD_ARDUINO 

(for arduino board version make from 2009 and after)


#Contains: 
the KiCAD footprint for some "ARDUINO" board:

ARDUINO_FIO_V22				Footprint contain only pin connectors. 
ARDUINO_FIO_V22_CONTOUR			Footprint contain pin connectors, board contour and position (VCC, USB, ISP, RESET button).
ARDUINO_FIO_V22_ELL			Footprint with large pads.
ARDUINO_FIO_V22_CONTOUR-ELL		Footprint with large pads.
ARDUINO_MEGA_2560_R3			Footprint contain only pin connectors and holes.
ARDUINO_MEGA_2560_R3_CONTOUR 		Footprint contain pin connectors, board contour and position (VCC, USB, ISP, RESET button).
ARDUINO_MEGA_2560_R3_ELL		Footprint with large pads.
ARDUINO_MEGA_2560_R3_CONTOUR_ELL	Footprint with large pads.
ARDUINO_MICRO_R3			Footprint contain only pin connectors and holes.
ARDUINO_MICRO_R3_CONTOUR		Footprint contain pin connectors, board contour and position (USB, ISP, RESET button).
ARDUINO_MICRO_R3_ELL			Footprint with large pads.
ARDUINO_MICRO_R3_CONTOUR_ELL		Footprint with large pads.
ARDUINO_MINI_PRO_V10			Footprint contain only pin connectors.
ARDUINO_MINI_PRO_V10_CONTOUR		Footprint contain pin connectors, board contour and position (RESET button).
ARDUINO_MINI_PRO_V10_ELL		Footprint with large pads.
ARDUINO_MINI_PRO_V10_CONTOUR_ELL	Footprint with large pads.
ARDUINO_MINI_V5				Footprint contain only pin connectors.
ARDUINO_MINI_V5_CONTOUR			Footprint contain pin connectors, board contour and position (RESET button).	
ARDUINO_MINI_V5_ELL			Footprint with large pads.	
ARDUINO_MINI_V5_CONTOUR_ELL		Footprint with large pads.
ARDUINO_NANO_2				Footprint contain only pin connectors and holes.
ARDUINO_NANO_2_CONTOUR			Footprint contain pin connectors, board contour and position (USB, ISP, RESET button).
ARDUINO_NANO_2_ELL			Footprint with large pads.	
ARDUINO_NANO_2_CONTOUR_ELL		Footprint with large pads.
ARDUINO_as_UNO_R3			Footprint contain only pin connectors and holes.
ARDUINO_as_UNO_R3_CONTOUR		Footprint contain pin connectors, board contour and position (VCC, USB, ISP, RESET button).
ARDUINO_as_UNO_R3_ELL			Footprint with large pads.
ARDUINO_as_UNO_R3_CONTOUR_ELL		Footprint with large pads.



/!\IMPORTANT: use directory "SHIELD_ARDUINO.pretty" if your have a new KICAD version (version >= BZR 5000).
Else if your have an old KICAD version use "SHIELD_ARDUINO.mod" /!\ 

GND/1, GND/2 ou TX/1, TX/2 ou 5V/1, 5V/2 ect... as the same in your board but it was separate in these modules.
Just for more free during your construction.

The postion of connectors (VCC, USB, ISP and reset) and holes are there for information purposes.
Adujst your footprint what do you want. 

Be careful the library may contain errors.



(Partie en français)

/!\Attention/!\ 

Les pins marquées GND/1, GND/2 ou TX/1, TX/2 ou 5V/1, 5V/2 ect... sont connectées 
entre elles sur les cartes officielles mais séparées sur ces modules. Cela vous laisse plus de liberté. 

Utiliser le répertoire "SHIELD_ARDUINO.pretty" si votre version de kicad est supérieur ou égale à la
"BZR 5110".
Sinon utiliser "SHIELD_ARDUINO.mod".

Les empreintes type "arduino_board_contour" contiennent le contour des cartes.
Mais aussi l'emplacement des trous, du connecteur "ISP", du bouton "RESET", des connecteurs 
"USB" et de l'alimentation si il y en a. 
Ils sont tracés avec des traits d'une épaisseur de "0.01mm" alors que les contours sont en "0.15mm".

Le but étant de vous donnez un maximum d'informations et d'ajuster selon vos besoins. 

Vérifier la version de votre carte et la présence des pins. 
Ajuster en conséquence.

La librairie peut contenir des erreurs alors vérifier deux fois.



Author:	 Jonathan Iapicco / France / 27 nov. 2014
License: GNU-GPL

