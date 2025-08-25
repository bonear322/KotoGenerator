# KotoGenerator
Технологии: C, libxml2, libzip

Кодогенератор, который преобразует математические выражений из DOCX-документа в функции на языке программирования C.

Передача входных параметров происходит через командную строку.
Кодогенератор содержит 4 обязательных параметра:
- -inputf – содержит относительный или абсолютный путь до входного файла.
- -outputf – содержит относительный или абсолютный путь до каталога с файлом вывода.
- -var_type – содержит тип переменных функций, которые будут созданы. Может быть float или double.
- -line_elem_count – содержит количество секций математического выражения в одной строке функции. Может быть нулем или больше нуля. При нуле всё математическое выражение записывается в одну строку в функции.<br>
И 1 дополнительный параметр:
- -word_trig_notat – указывает кодогенератору, что выражения из входного файла записаны в стандартной WORD-нотации тригонометрических функций. Если выражения записаны в классической нотации, то этого параметра не должно быть с строке с параметрами.

## Работа программы

**Пример 1.**
<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/input1.JPG"/>
</p>
<p align="center">Входной файл input1</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/cmd1.JPG"/>
</p>
<p align="center">Ввод параметров кодогенератора</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/output1.JPG"/>
</p>
<p align="center">Файл вывода output1</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/cmd1_2.JPG"/>
</p>
<p align="center">Запуск с иными параметрами кодогенератора</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/output1_2.JPG"/>
</p>
<p align="center">Файл вывода output1</p>
<br>

**Пример 2.**
<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/input2.JPG"/>
</p>
<p align="center">Входной файл input2</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/cmd2.JPG"/>
</p>
<p align="center">Ввод параметров кодогенератора</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/output2.JPG"/>
</p>
<p align="center">Файл вывода output2</p>
<br>

**Обработка ошибок 1.**
<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/error_input1.JPG"/>
</p>
<p align="center">Входной файл input1</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/error_output1.JPG"/>
</p>
<p align="center">Сообщение об ошибке</p>
<br>

**Обработка ошибок 2.**
<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/error_input2.JPG"/>
</p>
<p align="center">Входной файл input1</p>
<br>

<p align="center">
	<img src="https://github.com/bonear322/KotoGenerator/blob/main/screenshots/error_output2.JPG"/>
</p>
<p align="center">Сообщение об ошибке</p>
<br>


