/*
    using 'arm' to stop the adc/display of values.
    and then modifying a parameter works better than reset.


*/
#if 0


oct 31. 2023.
    got adc on. but have problem with offset.

configuration

- using lt1021/7V.  1ppm Vpp 0.1-10Hz.   as reference.
- lsk389 for amp.        will revert back to baseline jfe2140 when purchase more.

- adc resistors. 40k,40k,50k  ref,ref,sig.
- get around 1.2uV. stddev. when have monitor, and scope lead, and dmm connection attached.


sampling ref-lo via the azmux at 10nplc

gain 1x
> reset; azero off; nplc 10; himux gnd ; azmux ref-lo; gain 1;   trig;

app counts  2022723 1977521 823 4000000  sample 0.000,001,6V   mean(10) 0.45uV, stddev(10) 0.91uV,
app counts  2022723 1977521 829 4000000  sample 0.000,001,1V   mean(10) 0.60uV, stddev(10) 0.87uV,
app counts  2022723 1977521 821 4000000  sample 0.000,001,8V   mean(10) 0.85uV, stddev(10) 0.83uV,
app counts  2022723 1977521 835 4000000  sample 0.000,000,5V   mean(10) 0.96uV, stddev(10) 0.67uV,
app counts  2022723 1977521 836 4000000  sample 0.000,000,4V   mean(10) 0.80uV, stddev(10) 0.59uV,
app counts  2022723 1977521 836 4000000  sample 0.000,000,4V   mean(10) 0.75uV, stddev(10) 0.61uV,
app counts  2022723 1977521 823 4000000  sample 0.000,001,6V   mean(10) 0.83uV, stddev(10) 0.67uV,
app counts  2022723 1977521 833 4000000  sample 0.000,000,7V   mean(10) 0.83uV, stddev(10) 0.67uV,

> gain 10; trig;

app counts  2024593 1975573 577 4000000  sample 0.016,547,1V   mean(10) 16548.11uV, stddev(10) 1.24uV,
app counts  2024593 1975573 573 4000000  sample 0.016,547,5V   mean(10) 16547.89uV, stddev(10) 1.09uV,
app counts  2024593 1975573 573 4000000  sample 0.016,547,5V   mean(10) 16547.61uV, stddev(10) 0.69uV,
app counts  2024593 1975573 562 4000000  sample 0.016,548,6V   mean(10) 16547.73uV, stddev(10) 0.75uV,
app counts  2024593 1975573 564 4000000  sample 0.016,548,4V   mean(10) 16547.76uV, stddev(10) 0.78uV,


> gain 100; trig;

app counts  2046924 1953320 313 4000000  sample 0.209,448,3V   mean(10) 209462.31uV, stddev(10) 11.08uV,
app counts  2046924 1953320 383 4000000  sample 0.209,441,4V   mean(10) 209458.91uV, stddev(10) 11.82uV,
app counts  2046924 1953320 427 4000000  sample 0.209,437,0V   mean(10) 209454.94uV, stddev(10) 11.83uV,
app counts  2046924 1953320 433 4000000  sample 0.209,436,4V   mean(10) 209451.29uV, stddev(10) 11.29uV,
app counts  2046924 1953320 463 4000000  sample 0.209,433,5V   mean(10) 209447.72uV, stddev(10) 10.63uV,
app counts  2046924 1953320 419 4000000  sample 0.209,437,8V   mean(10) 209444.89uV, stddev(10) 8.80uV,
app counts  2046924 1953320 490 4000000  sample 0.209,430,8V   mean(10) 209442.11uV, stddev(10) 8.36uV,

0.2V. indicates jfet Vos. 2mV. for lsk389


sampling ref-hi at 1x gain
> reset; azero off; nplc 10; himux ref-hi ; azmux pcout; gain 1;   trig;

app counts 2831737 1168481 951 4000000  sample 6.999,999,2V   mean(10) 6999998.27uV, stddev(10) 0.80uV,
app counts 2831737 1168481 961 4000000  sample 6.999,998,3V   mean(10) 6999998.37uV, stddev(10) 0.72uV,
app counts 2831737 1168481 966 4000000  sample 6.999,997,8V   mean(10) 6999998.36uV, stddev(10) 0.73uV,
app counts 2831737 1168481 967 4000000  sample 6.999,997,7V   mean(10) 6999998.38uV, stddev(10) 0.71uV,
app counts 2831737 1168481 971 4000000  sample 6.999,997,3V   mean(10) 6999998.37uV, stddev(10) 0.72uV,
app counts 2831737 1168481 963 4000000  sample 6.999,998,1V   mean(10) 6999998.38uV, stddev(10) 0.72uV,
app counts 2831737 1168481 966 4000000  sample 6.999,997,8V   mean(10) 6999998.21uV, stddev(10) 0.61uV,

- noise is about the same. as sampling ref-lo. where would expect the noisy reference to be a bit higher.


sampling ref-lo at 1nplc.

> reset; azero off; nplc 1; himux gnd ; azmux ref-lo; gain 1;   trig;

app counts 202303 197763 917 400000  sample 0.000,014,8V   mean(10) 16.44uV, stddev(10) 2.10uV,
app counts 202303 197763 918 400000  sample 0.000,013,8V   mean(10) 16.05uV, stddev(10) 2.20uV,
app counts 202303 197763 913 400000  sample 0.000,018,7V   mean(10) 16.05uV, stddev(10) 2.20uV,
app counts 202303 197763 916 400000  sample 0.000,015,8V   mean(10) 15.76uV, stddev(10) 1.99uV,
app counts 202303 197763 913 400000  sample 0.000,018,7V   mean(10) 15.95uV, stddev(10) 2.18uV,
app counts 202303 197763 915 400000  sample 0.000,016,7V   mean(10) 16.24uV, stddev(10) 2.06uV,


  notice issue  - with 20mV offset.
    - due to poor calibration model. param=4.
    - or DA on non tdk capacitor.  or other poor adc configuration?.

    note - no az switching and therefore no switch charge-injection.



#endif






