<h1>Лабораторная работа No 1. Знакомство с Linux/Unix и средой программирования</h1>

<h2>Введение</h2>
<p>Программа dirwalk, сканирующая файловую систему и выводящая
информацию в соответствии с опциями программы.</p>


<h2>Компиляция</h2>
<p>Для компиляции программы выполните следующие команды:</p>
<code>git clone https://github.com/Hannah-Kaliada/OSaSP.git</code><br>
<code>cd OSaSP/lab1</code><br>
<code>gcc -o dirwalk dirwalk.c</code><br>
<code>./dirwalk -ldfs /path/to/directory/for/scan</code> <br>
<ul>
    <li>Для обработки опций использована библиотека <code>getopt(3)</code>.</li>
    <li>Программа поддерживает опции <code>-l</code>, <code>-d</code>, <code>-f</code>, <code>-s</code>.</li>
    <p>Допустимые опции:</p>
<ul>
    <li><code>-l</code>: только символические ссылки.</li>
    <li><code>-d</code>: только каталоги.</li>
    <li><code>-f</code>: только файлы.</li>
    <li><code>-s</code>: сортировать вывод в соответствии с <code>LC_COLLATE</code>.</li>
</ul>
    <li>Если опции не указаны, выводятся каталоги, файлы и символические ссылки.</li>
</ul>


