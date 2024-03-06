<h1>Лабораторная работа No 1. Знакомство с Linux/Unix и средой программирования</h1>

<h2>Введение</h2>
<p>Программа dirwalk, сканирующая файловую систему и выводящая
информацию в соответствии с опциями программы.</p>

<h2>Компиляция</h2>
<p>Для компиляции программы выполните следующие команды:</p>
<pre>
<code>git clone https://github.com/Hannah-Kaliada/OSaSP.git</code>
<code>cd OSaSP/lab1</code>
<code>gcc -o dirwalk dirwalk.c</code>
<code>./dirwalk -ldfs /path/to/directory/for/scan</code>
</pre>

<h2>Режимы работы</h2>

<p>Программа <code>dirwalk</code> поддерживает два режима работы, управляемых через опцию <code>mode</code> при компиляции.</p>
<ol>
    <li><strong>Режим отладки (<code>mode=debug</code>):</strong></li>
    <pre><code>make mode=debug</code></pre>
    <li>Режим выпуска (<code>mode=release</code>):</strong></li>
    <pre><code>make mode=release</code></pre>
</ol>
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


