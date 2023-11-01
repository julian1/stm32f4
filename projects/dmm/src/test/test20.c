/*
    using 'arm' to stop the adc/display of values.
    and then modifying a parameter works better than reset.


*/
#if 0


oct 31. 2023.
    got adc on. in non-az mode.

  comments

configuration


- ref lt1021/7V.  1ppm Vpp 0.1-10Hz.   as reference.
- amp lsk389 .     will revert to baseline jfe2140 when get some more.
- adc resistors. 40k,40k,50k  (+ref,-ref,sig). adc - cant find tdk caps.  using unknown c0g . enough for test.
      adc same as in 7.5 digit voltmeter. but using 4053 for integrator reset.
- supply - bench power supply
- calbration using nominal 7V. of lt1021.

 - all measurements are with az off. (still need a bit more code for az).

- noise seems higher than want. due to lt1021? albeit adc ref currents should cancel noise,  when sample LO/0V.
    or perhaps from slope amp.
    or lack of shielding
    or supply.
    - get extra > 1uV. stddev. if any ground referenced connection - programmer, monitor, scope or dmm leads, is attached inguard/isolated .


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

0.2V. offset at 100x gain, indicates jfet Vos. 2mV. for lsk389

With 100x gain,  and rms noise of ~= 10uV. indicates  amplifier is 100nV RMS ?.


sampling ref-hi at 1x gain
> reset; azero off; nplc 10; himux ref-hi ; azmux pcout; gain 1;   trig;
app counts 2831737 1168481 951 4000000  sample 6.999,999,2V   mean(10) 6999998.27uV, stddev(10) 0.80uV,
app counts 2831737 1168481 961 4000000  sample 6.999,998,3V   mean(10) 6999998.37uV, stddev(10) 0.72uV,
app counts 2831737 1168481 966 4000000  sample 6.999,997,8V   mean(10) 6999998.36uV, stddev(10) 0.73uV,
app counts 2831737 1168481 967 4000000  sample 6.999,997,7V   mean(10) 6999998.38uV, stddev(10) 0.71uV,
app counts 2831737 1168481 971 4000000  sample 6.999,997,3V   mean(10) 6999998.37uV, stddev(10) 0.72uV,
app counts 2831737 1168481 963 4000000  sample 6.999,998,1V   mean(10) 6999998.38uV, stddev(10) 0.72uV,
app counts 2831737 1168481 966 4000000  sample 6.999,997,8V   mean(10) 6999998.21uV, stddev(10) 0.61uV,

- noise about the same as for ref-lo.
    while might expect the lt1021 reference noise - to show a bit more.


sampling ref-lo at 1nplc.
> reset; azero off; nplc 1; himux gnd ; azmux ref-lo; gain 1;   trig;
app counts 202303 197763 917 400000  sample 0.000,014,8V   mean(10) 16.44uV, stddev(10) 2.10uV,
app counts 202303 197763 918 400000  sample 0.000,013,8V   mean(10) 16.05uV, stddev(10) 2.20uV,
app counts 202303 197763 913 400000  sample 0.000,018,7V   mean(10) 16.05uV, stddev(10) 2.20uV,
app counts 202303 197763 916 400000  sample 0.000,015,8V   mean(10) 15.76uV, stddev(10) 1.99uV,
app counts 202303 197763 913 400000  sample 0.000,018,7V   mean(10) 15.95uV, stddev(10) 2.18uV,
app counts 202303 197763 915 400000  sample 0.000,016,7V   mean(10) 16.24uV, stddev(10) 2.06uV,


  notice issue  - with ~= 16uV offset. at 1nplc versus 10nplc

    issues,
      - no/not enough low nplc data - in calibration model
      - da on cap non TDK cap/  .  could try 1206 cap. ok.
      - or 4 variable regression model.
    other
      - board/near adc not clean?
      - or slope-amp lt1357, or compound divider,    or not opa140.
      - or calibration routing - no variation in var/fix.
      - integrator reset circuit / charge?.


    note - no az switching and therefore no az switch charge-injection.


ref-lo at 10nplc seems lower noise now.
> reset; azero off; nplc 10; himux gnd ; azmux ref-lo; gain 1;   trig;
app counts 2022723 1977521 795 4000000  sample 0.000,002,9V   mean(10) 1.58uV, stddev(10) 0.62uV,
app counts 2022723 1977521 804 4000000  sample 0.000,002,1V   mean(10) 1.60uV, stddev(10) 0.64uV,
app counts 2022723 1977521 804 4000000  sample 0.000,002,1V   mean(10) 1.60uV, stddev(10) 0.63uV,
app counts 2022723 1977521 809 4000000  sample 0.000,001,6V   mean(10) 1.67uV, stddev(10) 0.56uV,
app counts 2022723 1977521 804 4000000  sample 0.000,002,1V   mean(10) 1.77uV, stddev(10) 0.53uV,
app counts 2022723 1977521 812 4000000  sample 0.000,001,3V   mean(10) 1.74uV, stddev(10) 0.55uV,
app counts 2022723 1977521 800 4000000  sample 0.000,002,4V   mean(10) 1.88uV, stddev(10) 0.54uV,


loop3. - with 4 model.
stderr(V) 1.82uV  (nplc10)
stderr(V) 2.01uV  (nplc10)
stderr(V) 1.73uV  (nplc10)

reset; azero off; nplc 10; dcv-source 1 ;  himux dcv-source ; azmux pcout; gain 1;   trig;

problem with negative voltage dcsource. weird.
reset; azero off; nplc 10; dcv-source -10 ;  himux dcv-source ; azmux pcout; gain 1;   trig

app counts 890277 3110097 1362 4000000  sample -9.798,975,6V   mean(10) -9796352.68uV, stddev(10) 1597.79uV,
app counts 890438 3109884 663 4000000  sample -9.797,286,3V   mean(10) -9796476.48uV, stddev(10) 1619.40uV,
app counts 890438 3109832 332 4000000  sample -9.797,026,2V   mean(10) -9796707.72uV, stddev(10) 1500.46uV,

  -0.1 and   -1V dc source looks ok. - so it could be a headroom issue - with amplifier ?
  - or headroom issue - with adc.


-----------


EXTR.
noise gets very low. sampling dcv-source hi. - when it's only the rundown count.
  so it's the effect of different number of up/down phases - and then different coefficients.

> reset; azero off; nplc 10; dcv-source 10 ;  himux dcv-source ; azmux pcout; gain 1;   trig
app counts 3164945 835351 966 4000000  sample 9.882,696,1V   mean(10) 9882695.80uV, stddev(10) 0.19uV,
app counts 3164945 835351 967 4000000  sample 9.882,696,1V   mean(10) 9882695.83uV, stddev(10) 0.21uV,
app counts 3164945 835351 969 4000000  sample 9.882,695,9V   mean(10) 9882695.86uV, stddev(10) 0.19uV,
app counts 3164945 835351 971 4000000  sample 9.882,695,7V   mean(10) 9882695.82uV, stddev(10) 0.18uV,
app counts 3164945 835351 971 4000000  sample 9.882,695,7V   mean(10) 9882695.79uV, stddev(10) 0.18uV,
app counts 3164945 835351 974 4000000  sample 9.882,695,4V   mean(10) 9882695.75uV, stddev(10) 0.23uV,
app counts 3164945 835351 970 4000000  sample 9.882,695,8V   mean(10) 9882695.77uV, stddev(10) 0.22uV,
app counts 3164945 835351 968 4000000  sample 9.882,696,0V   mean(10) 9882695.79uV, stddev(10) 0.22uV,
app counts 3164945 835351 968 4000000  sample 9.882,696,0V   mean(10) 9882695.81uV, stddev(10) 0.23uV,
app counts 3164945 835351 969 4000000  sample 9.882,695,9V   mean(10) 9882695.83uV, stddev(10) 0.22uV,
app counts 3164945 835351 970 4000000  sample 9.882,695,8V   mean(10) 9882695.79uV, stddev(10) 0.19uV,
app counts 3164945 835351 968 4000000  sample 9.882,696,0V   mean(10) 9882695.78uV, stddev(10) 0.18uV,

actually could be going out of bounds and being sofened??
at 1nplc.

> reset; azero off; nplc 1; dcv-source 10 ;  himux dcv-source ; azmux pcout; gain 1;   trig
app counts 316534 83558 893 400000  sample 9.882,579,9V   mean(10) 9882578.38uV, stddev(10) 1.89uV,
app counts 316534 83558 899 400000  sample 9.882,574,1V   mean(10) 9882577.61uV, stddev(10) 1.89uV,
app counts 316534 83558 896 400000  sample 9.882,577,0V   mean(10) 9882577.61uV, stddev(10) 1.89uV,
app counts 316534 83558 895 400000  sample 9.882,578,0V   mean(10) 9882577.71uV, stddev(10) 1.88uV,
app counts 316534 83558 894 400000  sample 9.882,579,0V   mean(10) 9882577.80uV, stddev(10) 1.92uV,
app counts 316534 83558 899 400000  sample 9.882,574,1V   mean(10) 9882577.61uV, stddev(10) 2.19uV,


- EXTR. have the same issue - super choppy.  if pos and neg counts are changing.  but ok if only rd counts.
    - so calibration constants are at issue. could be board clenliness, DA. anything.


# after changin fpga code to perhaps to support single adc. multiple controllers.
stderr(V) 1.50uV  (nplc10)
stderr(V) 1.58uV  (nplc10)

nose seems lower.

> reset; azero off; nplc 10; himux ref-lo ; azmux pcout; gain 1;   trig
app counts 2022723 1977521 839 4000000  sample -0.000,001,8V   mean(10) -2.88uV, stddev(10) 0.60uV,
app counts 2022723 1977521 846 4000000  sample -0.000,002,5V   mean(10) -2.78uV, stddev(10) 0.57uV,
app counts 2022723 1977521 848 4000000  sample -0.000,002,7V   mean(10) -2.68uV, stddev(10) 0.46uV,
app counts 2022723 1977521 850 4000000  sample -0.000,002,9V   mean(10) -2.66uV, stddev(10) 0.44uV,
app counts 2022723 1977521 846 4000000  sample -0.000,002,5V   mean(10) -2.63uV, stddev(10) 0.44uV,
app counts 2022723 1977521 854 4000000  sample -0.000,003,3V   mean(10) -2.67uV, stddev(10) 0.48uV,
app counts 2022723 1977521 843 4000000  sample -0.000,002,2V   mean(10) -2.63uV, stddev(10) 0.50uV,
app counts 2022723 1977521 847 4000000  sample -0.000,002,6V   mean(10) -2.69uV, stddev(10) 0.45uV,




nov 1. 2023.


first az. jjjjjjjjj

reset; azero on; nplc 10; himux ref-hi ; azmux ref-lo ; gain 1;   trig

app counts 2831737 1168481 936 4000000  sample 6.999,994,5V   mean(10) 3500006.80uV, stddev(10) 3689311.31uV,
app counts 2022723 1977521 584 4000000  sample 0.000,018,9V   mean(10) 3500006.90uV, stddev(10) 3689311.21uV,
app counts 2831737 1168481 928 4000000  sample 6.999,995,3V   mean(10) 3500007.02uV, stddev(10) 3689311.34uV,
app counts 2022723 1977521 572 4000000  sample 0.000,020,1V   mean(10) 3500007.20uV, stddev(10) 3689311.15uV,
app counts 2831737 1168481 935 4000000  sample 6.999,994,6V   mean(10) 3500007.07uV, stddev(10) 3689311.01uV,
app counts 2022723 1977521 575 4000000  sample 0.000,019,8V   mean(10) 3500007.14uV, stddev(10) 3689310.94uV,
app counts 2831737 1168481 923 4000000  sample 6.999,995,8V   mean(10) 3500007.26uV, stddev(10) 3689311.06uV,


- problem at 1nplc,  with large offset.   100uV..
    issue with charge? or az switch timing?

    > reset; azero on; nplc 1; himux ref-hi ; azmux ref-lo ; gain 1;   trig

      app counts 283274 116922 1271 400000  sample 6.999,897,6V   mean(10) 3500013.99uV, stddev(10) 3689200.03uV,
      app counts 202303 197763 810 400000  sample 0.000,129,9V   mean(10) 3500013.99uV, stddev(10) 3689200.03uV,
      app counts 283274 116922 1272 400000  sample 6.999,896,6V   mean(10) 3500014.19uV, stddev(10) 3689200.24uV,
      app counts 202303 197763 803 400000  sample 0.000,136,9V   mean(10) 3500014.59uV, stddev(10) 3689199.82uV,
      app counts 283274 116922 1271 400000  sample 6.999,897,6V   mean(10) 3500014.99uV, stddev(10) 3689200.24uV,


--------

AZERO - with actual subtraction. at 1nplc.
But problem with 100uV. offset. indicating somthing isn't right.
reset; azero on; nplc 1; himux ref-hi ; azmux ref-lo ; gain 1;   trig

app counts 283274 116922 1272 400000  sample 6.999,863,2V   mean(10) 6999761.38uV, stddev(10) 1.55uV,
app counts 202303 197763 805 400000   sample 0.000,100,4V   mean(10) 6999761.38uV, stddev(10) 1.55uV,
app counts 283274 116922 1273 400000  sample 6.999,862,2V   mean(10) 6999761.58uV, stddev(10) 1.67uV,
app counts 202303 197763 807 400000   sample 0.000,098,4V   mean(10) 6999761.58uV, stddev(10) 1.67uV,
app counts 283274 116922 1273 400000  sample 6.999,862,2V   mean(10) 6999761.98uV, stddev(10) 1.39uV,
app counts 202303 197763 804 400000   sample 0.000,101,4V   mean(10) 6999761.98uV, stddev(10) 1.39uV,
app counts 283274 116922 1271 400000  sample 6.999,864,2V   mean(10) 6999762.27uV, stddev(10) 1.55uV,


----------------------------------------------------------------------
but also offset. with nplc=10.  between non-az and az mode.

calibration done at nplc=10.
so the aperture is the same for cal, az, non-az samples.

> reset; azero off; nplc 10; himux ref-hi ; azmux pcout ; gain 1;   trig
app counts 2831737 1168481    830 4000000  no-az sample 7.000,000,2V   mean(10) 6999999.07uV, stddev(10) 0.84uV,
app counts 2831737 1168481    820 4000000  no-az sample 7.000,001,2V   mean(10) 6999999.35uV, stddev(10) 1.04uV,
app counts 2831737 1168481    852 4000000  no-az sample 6.999,998,1V   mean(10) 6999999.28uV, stddev(10) 1.10uV,
app counts 2831737 1168481    830 4000000  no-az sample 7.000,000,2V   mean(10) 6999999.46uV, stddev(10) 1.08uV,
app counts 2831737 1168481    843 4000000  no-az sample 6.999,998,9V   mean(10) 6999999.43uV, stddev(10) 1.10uV,
app counts 2831737 1168481    834 4000000  no-az sample 6.999,999,8V   mean(10) 6999999.59uV, stddev(10) 1.02uV,
app counts 2831737 1168481    857 4000000  no-az sample 6.999,997,6V   mean(10) 6999999.32uV, stddev(10) 1.16uV,


> reset; azero on; nplc 10; himux ref-hi ; azmux ref-lo ; gain 1;   trig
app counts 2022723 1977521    677 4000000  az sample 6.999,977,0V (0.000,007,9V, 6.999,985,1V)   mean(10) 6999976.62uV, stddev(10) 0.80uV,
app counts 2831737 1168481    979 4000000  az sample 6.999,977,6V (0.000,007,9V, 6.999,985,6V)   mean(10) 6999976.80uV, stddev(10) 0.79uV,
app counts 2022723 1977521    658 4000000  az sample 6.999,976,8V (0.000,009,7V, 6.999,985,6V)   mean(10) 6999976.89uV, stddev(10) 0.72uV,
app counts 2831737 1168481    970 4000000  az sample 6.999,977,7V (0.000,009,7V, 6.999,986,5V)   mean(10) 6999977.00uV, stddev(10) 0.76uV,
app counts 2022723 1977521    668 4000000  az sample 6.999,977,3V (0.000,008,8V, 6.999,986,5V)   mean(10) 6999977.01uV, stddev(10) 0.76uV,
app counts 2831737 1168481    960 4000000  az sample 6.999,978,2V (0.000,008,8V, 6.999,987,5V)   mean(10) 6999977.05uV, stddev(10) 0.81uV,
app counts 2022723 1977521    667 4000000  az sample 6.999,978,7V (0.000,008,9V, 6.999,987,5V)   mean(10) 6999977.14uV, stddev(10) 0.94uV,
app counts 2831737 1168481    984 4000000  az sample 6.999,976,3V (0.000,008,9V, 6.999,985,2V)   mean(10) 6999977.17uV, stddev(10) 0.90uV,


eg. the lo sample (ref-lo) is shifted up 9uV. and the hi sample  (ref-hi) is down 15uV.

need to give amp. longer to settle?

are we sure we are not sampling the wrong lo. somehow?
board clenliness. DA. would not expect to matter.


nice. at least doesn't matter if mux ref-lo through himux or azmux.

#endif







