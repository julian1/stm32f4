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

  > reset; azero off; nplc 10; himux ref-hi ; azmux pcout ; pc signal ;  gain 1;   trig
  app counts 2831737 1168481    835 4000000  no-az sample 6.999,998,7V   mean(10) 6.9999984V, stddev(10) 0.61uV,
  app counts 2831737 1168481    842 4000000  no-az sample 6.999,998,0V   mean(10) 6.9999984V, stddev(10) 0.60uV,
  app counts 2831737 1168481    830 4000000  no-az sample 6.999,999,1V   mean(10) 6.9999984V, stddev(10) 0.63uV,
  app counts 2831737 1168481    839 4000000  no-az sample 6.999,998,3V   mean(10) 6.9999984V, stddev(10) 0.63uV,
  app counts 2831737 1168481    845 4000000  no-az sample 6.999,997,7V   mean(10) 6.9999984V, stddev(10) 0.65uV,
  app counts 2831737 1168481    827 4000000  no-az sample 6.999,999,4V   mean(10) 6.9999984V, stddev(10) 0.59uV,
  app counts 2831737 1168481    849 4000000  no-az sample 6.999,997,3V   mean(10) 6.9999983V, stddev(10) 0.68uV,
  app counts 2831737 1168481    847 4000000  no-az sample 6.999,997,5V   mean(10) 6.9999983V, stddev(10) 0.69uV,


  > reset; azero on; nplc 10; himux ref-hi ; azmux ref-lo ; gain 1;   trig
  app counts 2022723 1977521    666 4000000  az sample 6.999,976,9V (0.000,009,8V, 6.999,986,0V)   mean(10) 6.9999767V, stddev(10) 0.88uV,
  app counts 2831737 1168481    976 4000000  az sample 6.999,976,5V (0.000,009,8V, 6.999,985,6V)   mean(10) 6.9999768V, stddev(10) 0.79uV,
  app counts 2022723 1977521    666 4000000  az sample 6.999,975,8V (0.000,009,8V, 6.999,985,6V)   mean(10) 6.9999768V, stddev(10) 0.79uV,
  app counts 2831737 1168481    977 4000000  az sample 6.999,975,7V (0.000,009,8V, 6.999,985,5V)   mean(10) 6.9999768V, stddev(10) 0.74uV,
  app counts 2022723 1977521    664 4000000  az sample 6.999,975,6V (0.000,010,0V, 6.999,985,5V)   mean(10) 6.9999767V, stddev(10) 0.82uV,
  app counts 2831737 1168481    976 4000000  az sample 6.999,975,7V (0.000,010,0V, 6.999,985,6V)   mean(10) 6.9999766V, stddev(10) 0.88uV,
  app counts 2022723 1977521    663 4000000  az sample 6.999,975,5V (0.000,010,1V, 6.999,985,6V)   mean(10) 6.9999764V, stddev(10) 0.84uV,
  app counts 2831737 1168481    975 4000000  az sample 6.999,975,6V (0.000,010,1V, 6.999,985,7V)   mean(10) 6.9999763V, stddev(10) 0.82uV,


eg. the lo sample (ref-lo) is shifted up 9uV. and the hi sample  (ref-hi) is down 15uV.

need to give amp. longer to settle?

are we sure we are not sampling the wrong lo. somehow?
board clenliness. DA. would not expect to matter.
  - actually integrator cap DA - could affect . because integrator voltage range will change - depending on what we are sampling..




nice. at least doesn't matter if mux ref-lo through himux or azmux.

//////////////////////////////////////////
oct 2.

we have 20uV. offset just sampling ref-lo in non-az.  for 1nplc versus 10nplc.

  > reset; azero off; nplc 1; himux ref-lo ; azmux pcout ; pc signal ;  gain 1;   trig
  app counts 202303 197763    907 400000  no-az sample 0.000,021,9V   mean(10) 0.0000246V, stddev(10) 2.14uV,

  > reset; azero off; nplc 10; himux ref-lo ; azmux pcout ; pc signal ;  gain 1;   trig;
  app counts 2022723 1977521    735 4000000  no-az sample -0.000,003,8V   mean(10) -0.0000027V, stddev(10) 0.68uV,

Since pc switch/ azmux switch is not switching. it indicates an error somewhere else.
perhaps aperture count issue.
da.
cleanliness.


Ok. got azero difference at least working. but not much improvement in noise.
- same as azero when sampling ref-hi.
- and not much different to non az

> reset; azero on; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1;   trig
  counts 2022723 1977521    833 4000000   (lo) az meas -0.000,001,3V (hi -0.000,000,3V) (lo 0.000,001,1V, 0.000,001,0V)   mean(10) -0.0000018V, stddev(10) 0.45uV,

>  reset; azero off; nplc 1; himux ref-lo ; azmux pcout ; pc signal ;  gain 1;   trig

//////////////


// changed the cap to TDK 1206 330p.. and we have the same problem . in non-az  1nplc v 10nplc.

> reset; azero off; nplc 10; himux ref-lo ; azmux pcout ; pc signal ;  gain 1;   trig
  counts 2022723 1977521    580 4000000  no-az meas -0.000,001,6V   mean(10) -0.0000026V, stddev(10) 0.55uV,

> reset; azero off; nplc 1; himux ref-lo ; azmux pcout ; pc signal ;  gain 1;   trig
  counts 202303 197763    923 400000  no-az meas -0.000,054,4V   mean(10) -0.0000549V, stddev(10) 1.62uV,

offset is worse 52uV.  after changing to tdk cap.  ????

So perhaps it is the calibration ? maybe previous - had different var/fix data?


cols = 4.
  counts 202303 197763    929 400000  no-az meas -0.000,152,1V   mean(10) -0.0001475V, stddev(10) 2.23uV,

add extra nplc data.
  even worse.

  stderr(V) 6.75uV  (nplc10)
  stderr(V) 6.94uV  (nplc10)

  counts 202303 197763    928 400000  no-az meas -0.000,107,9V   mean(10) -0.0001060V, stddev(10) 1.63uV,


very odd.
it really cant fit the data. assign good weights,
  azero case is ok. at 10nplc. but bad at 1nplc.
  something around start/ reset signal / or reset switch / or settle time.

at 1nplc ref-hi has positive contribution
  nplc 1    counts 283274 116896     1090 400000  no-az meas 7.000,216,2V   +ve
  nplc 5    counts 1415949 584199    612 2000000  no-az meas 7.000,037,8V
  nplc 10   counts 2831872 1168398   452 4000000  no-az meas 7.000,004,5V
  nplc 20   counts 5663744 2336796   1176 8000000  no-az meas 6.999,990,8V   -ve.

at 1nplc ref-lo has negative contribution.
  nplc 0.5 counts 101258 98988       360 200000  no-az meas -0.000,033,9V
  nplc 1   counts 202303 197763      927 400000  no-az meas -0.000,024,1V
  nplc 5   counts 1011437 988815    1197 2000000 no-az meas -0.000,007,8V
  nplc 10  counts 2022723 1977521    649 4000000 no-az meas -0.000,000,6V

  - the longer it runs/ larger aperture - the better - as bad thing is overwhelmed.
  - So. we need to try changing the 20k. reset resistor.
  - revert slope amp
  - use faster op for current sources .

  So that is interesting.





nov 3.
  with two variable model.  and checking the reset.   still have a 200uV. offset. at 1uV.
    it is like aperture is being wrongly used.


> reset; azero off; nplc 1; himux ref-hi; azmux pcout ; pc signal ;  gain 1;   trig
  counts  10002 283274 116896   1069 400001no-az meas 7.000,241,2V   mean(10) 7.0002419V, stddev(10) 1.79uV,

  changed the reset period. 20000.

  counts  20002 283274 116896   1050 400001no-az meas 7.000,259,8V   mean(10) 7.0002580V, stddev(10) 3.28uV,

  no difference.
  OK. good to know.


  change reset from 20k to 3.74k.

  calibration stderr=3.93uV.  cols = 4.

  counts  10002 283274 116896   1070 400001no-az meas 7.000,206,5V   mean(10) 7.0002077V, stddev(10) 2.01uV,

    OK. a bit lower...

  cols = 2.
  counts  10002 283274 116896   1067 400001no-az meas 7.000,242,6V   mean(10) 7.0002412V, stddev(10) 2.69uV,

  looks about the same.......

  -------------------


OK. changed the integrator voltage range.  by reducing fixn varn - and we get a much better result.

cols =  2

  stderr(V) 1.06uV  (nplc10)
  stderr(V) 0.69uV  (nplc10)

reset; azero off; nplc 10; himux ref-hi; azmux pcout ; pc signal ;  gain 1;   trig
counts  10002 2831605 1168529    737 4000001 no-az meas 7.000,000,6V   mean(10) 7.0000003V, stddev(10) 0.83uV,
counts  10002 2831605 1168529    740 4000001 no-az meas 7.000,000,3V   mean(10) 7.0000005V, stddev(10) 0.64uV,

reset; azero off; nplc 1; himux ref-hi; azmux pcout ; pc signal ;  gain 1;   trig
counts  10002 283237 116909    903 400001 no-az meas 6.999,998,0V   mean(10) 6.9999973V, stddev(10) 1.30uV,
counts  10002 283237 116909    905 400001 no-az meas 6.999,996,0V   mean(10) 6.9999972V, stddev(10) 1.36uV,
counts  10002 283237 116909    902 400001 no-az meas 6.999,998,9V   mean(10) 6.9999973V, stddev(10) 1.45uV,

  - only 2uV.

reset; azero off; nplc 10; himux ref-lo; azmux pcout ; pc signal ;  gain 1;   trig
counts  10002 2022507 1977610    897 4000001 no-az meas -0.000,003,4V   mean(10) -0.0000031V, stddev(10) 0.39uV,
counts  10002 2022507 1977610    897 4000001 no-az meas -0.000,003,4V   mean(10) -0.0000031V, stddev(10) 0.38uV,

reset; azero off; nplc 1; himux ref-lo; azmux pcout ; pc signal ;  gain 1;   trig
counts  10002 202317 197812    714 400001 no-az meas -0.000,004,4V   mean(10) -0.0000056V, stddev(10) 1.43uV,
counts  10002 202317 197812    717 400001 no-az meas -0.000,007,3V   mean(10) -0.0000058V, stddev(10) 1.53uV,


-
    so it's leakage through reset mux. at higher voltage.
    or op-amp headroom or something.
        OR DA on the cap with larger range.

        or difficulty with DA because slow slope is too slow.


loop3.
stderr(V) 0.86uV  (nplc10)

placing a loose tin can over top of pcb.
stderr(V) 0.65uV  (nplc10)
stderr(V) 0.64uV  (nplc10)

counts  10002 2022507 1977610    917 4000001 (lo)  (hi 7.000,000,2V) (lo -0.000,001,8V, -0.000,002,0V)az meas 7.000,002,1V   mean(10) 7.0000023V, stddev(10) 0.45uV,
counts  10002 2831605 1168529    760 4000001 (hi)  (hi 7.000,000,5V) (lo -0.000,001,8V, -0.000,002,0V)az meas 7.000,002,4V   mean(10) 7.0000023V, stddev(10) 0.45uV,
counts  10002 2022507 1977610    920 4000001 (lo)  (hi 7.000,000,5V) (lo -0.000,002,1V, -0.000,001,8V)az meas 7.000,002,4V   mean(10) 7.0000023V, stddev(10) 0.44uV,
counts  10002 2831605 1168529    761 4000001 (hi)  (hi 7.000,000,4V) (lo -0.000,002,1V, -0.000,001,8V)az meas 7.000,002,3V   mean(10) 7.0000022V, stddev(10) 0.33uV,
counts  10002 2022507 1977610    922 4000001 (lo)  (hi 7.000,000,4V) (lo -0.000,002,2V, -0.000,002,1V)az meas 7.000,002,6V   mean(10) 7.0000022V, stddev(10) 0.25uV,
counts  10002 2831605 1168529    761 4000001 (hi)  (hi 7.000,000,4V) (lo -0.000,002,2V, -0.000,002,1V)az meas 7.000,002,6V   mean(10) 7.0000022V, stddev(10) 0.27uV


improve can position

stderr(V) 0.51uV  (nplc10)
stderr(V) 0.60uV  (nplc10)


A lot of this is with a col = 3 model.



I spent a couple of days, investigating
There is an issue related to integrator voltage range/swing,
If too high, then cap DA, or reset-mux leakage, or op-amp IVR, or something else, prevents getting a reasonable calibration against the internal ref.
If the integrator swing is limited to around 10Vpp then it is ok
I am leaving it for the moment.

EXTR - It may just be a op-amp supply headroom issue. because integrator suppliies are +-15V. instead of +-17.5V  on old adc.

---



configuration -
ref lt1021/7V.
amplifier lsk389. will revert to jfe2140 as baseline, when get some more.
bench supply.
a biscuit tin lid covering the pcb helps to reduce noise.


The main column of interest is the last one,
10nplc noise is around 0.33uV RMS.  1nplc noise around 1.4uV RMS. I believe this is mostly from the adc input resistors.
reference noise in the adc, should most cancel, for a lo measurement/sample.


sample ref-lo with no amplifier gain,

az 10nplc
> reset; azero on; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002 2022507 1977610    891 4000001 (lo)  (hi -0.000,003,7V) (lo 0.000,003,8V, 0.000,003,7V) az meas -0.000,007,5V   mean(30) -0.0000077V, stddev(30) 0.31uV,
counts  10002 2022490 1977610    232 4000001 (hi)  (hi -0.000,004,6V) (lo 0.000,003,8V, 0.000,003,7V) az meas -0.000,008,3V   mean(30) -0.0000077V, stddev(30) 0.32uV,
counts  10002 2022507 1977610    891 4000001 (lo)  (hi -0.000,004,6V) (lo 0.000,003,8V, 0.000,003,8V) az meas -0.000,008,4V   mean(30) -0.0000078V, stddev(30) 0.33uV,
counts  10002 2022490 1977610    229 4000001 (hi)  (hi -0.000,004,3V) (lo 0.000,003,8V, 0.000,003,8V) az meas -0.000,008,1V   mean(30) -0.0000078V, stddev(30) 0.33uV,
counts  10002 2022507 1977610    889 4000001 (lo)  (hi -0.000,004,3V) (lo 0.000,004,0V, 0.000,003,8V) az meas -0.000,008,2V   mean(30) -0.0000078V, stddev(30) 0.33uV,


az 1nplc.
>  reset; azero on; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002 202317 197812    726 400001 (hi)  (hi -0.000,009,9V) (lo -0.000,001,1V, 0.000,001,8V) az meas -0.000,010,2V   mean(30) -0.0000071V, stddev(30) 1.28uV,
counts  10002 202317 197812    713 400001 (lo)  (hi -0.000,009,9V) (lo 0.000,002,7V, -0.000,001,1V) az meas -0.000,010,7V   mean(30) -0.0000072V, stddev(30) 1.41uV,
counts  10002 202317 197812    723 400001 (hi)  (hi -0.000,007,0V) (lo 0.000,002,7V, -0.000,001,1V) az meas -0.000,007,8V   mean(30) -0.0000072V, stddev(30) 1.39uV,
counts  10002 202317 197812    716 400001 (lo)  (hi -0.000,007,0V) (lo -0.000,000,2V, 0.000,002,7V) az meas -0.000,008,3V   mean(30) -0.0000071V, stddev(30) 1.37uV,
counts  10002 202317 197812    721 400001 (hi)  (hi -0.000,005,0V) (lo -0.000,000,2V, 0.000,002,7V) az meas -0.000,006,3V   mean(30) -0.0000071V, stddev(30) 1.38uV,


There is a -8uV difference when ref-lo is sampled from the himux versus the azmux - in az mode, regardless of nplc.
The ref-lo net/trace is kelvin sensed at the gnd pin of the lt1021.
The only adc count that changes (for 1nplc example) is the rundown count, so it is not a calculation artifact.

The difference is a bit large to be a thermocouple effect on ic pins/ or copper trace.
So I don't like this.
maybe switch charge-injection when the az mux switches between the lo/boot from the pc-switch to the ref-lo.
and/or distribution for different impedances of the mux paths?
But I still wouldn't expect this given that ref-lo is a low impedance input.

The other LO that is common to himux/himux2 and azmux is the star-lo.
So I should check to see if that shows the same issue.
Also there is the resistor R417 that can match/compensate the rds-on of the hi muxes.



for 10nplc no-az input noise is about the same as the az case.

> reset; azero off; nplc 10; himux ref-lo; azmux pcout ; pc signal ;  gain 1;  buffer 30; trig
counts  10002 2022490 1977610    228 4000001 no-az meas -0.000,004,2V   mean(30) -0.0000038V, stddev(30) 0.34uV,
counts  10002 2022490 1977610    225 4000001 no-az meas -0.000,003,9V   mean(30) -0.0000038V, stddev(30) 0.33uV,
counts  10002 2022490 1977610    232 4000001 no-az meas -0.000,004,6V   mean(30) -0.0000038V, stddev(30) 0.35uV,
counts  10002 2022490 1977610    230 4000001 no-az meas -0.000,004,4V   mean(30) -0.0000038V, stddev(30) 0.37uV,

There is a 3.8uV difference in non-az mode.
But this expected thermal variation (ref, op-amp Vos,resistors) from the calibration baseline point taken about 10-15mins earlier.



For the ampliifier, the ref-lo can be sampled with gain applied,

az, 1nplc, gain = 100.
>  reset; azero on; nplc 1; himux ref-lo ; azmux ref-lo ; gain 100; buffer 30;  trig
COUNTs  10002 204765 195449    840 400001 (lo)  (hi 0.207,785,5V) (lo 0.207,950,7V, 0.207,939,0V) az meas -0.000,159,4V   mean(30) -0.0001477V, stddev(30) 7.82uV,
counts  10002 204748 195449    246 400001 (hi)  (hi 0.207,803,0V) (lo 0.207,950,7V, 0.207,939,0V) az meas -0.000,141,8V   mean(30) -0.0001472V, stddev(30) 7.71uV,
counts  10002 204765 195449    830 400001 (lo)  (hi 0.207,803,0V) (lo 0.207,960,5V, 0.207,950,7V) az meas -0.000,152,6V   mean(30) -0.0001470V, stddev(30) 7.51uV,
counts  10002 204748 195449    258 400001 (hi)  (hi 0.207,791,3V) (lo 0.207,960,5V, 0.207,950,7V) az meas -0.000,164,3V   mean(30) -0.0001478V, stddev(30) 8.06uV,
counts  10002 204765 195449    839 400001 (lo)  (hi 0.207,791,3V) (lo 0.207,951,7V, 0.207,960,5V) az meas -0.000,164,8V   mean(30) -0.0001484V, stddev(30) 8.62uV,

note. amplifier Vos around 2mV.

no-az,  at 1nplc  gain = 100.
> reset; azero off; nplc 1; himux ref-lo; azmux pcout ; pc signal ;  gain 100;   trig
counts  10002 204748 195449    210 400001 no-az meas 0.207,838,1V   mean(30) 0.2078348V, stddev(30) 6.20uV,
counts  10002 204748 195449    211 400001 no-az meas 0.207,837,2V   mean(30) 0.2078352V, stddev(30) 6.00uV,
counts  10002 204748 195449    223 400001 no-az meas 0.207,825,5V   mean(30) 0.2078349V, stddev(30) 6.26uV,
counts  10002 204748 195449    216 400001 no-az meas 0.207,832,3V   mean(30) 0.2078343V, stddev(30) 5.66uV,
counts  10002 204765 195449    946 400001 no-az meas 0.207,847,3V   mean(30) 0.2078348V, stddev(30) 6.12uV,


For the 1nplc, gain=100x case, the az and no-az look similar for noise.
I am not sure how to interpret that.
Is the noise from the amplifier or 99k/1k feedback resistors, or the (amplified) white-noise of the resistance of the muxes/passives before the amplifier?
I kind of expected the az subtraction to cut-out flicker noise in a more observable way.
Or maybe it is evident, given that AZ mode only uses half the HI samples, but achieves similar variation.


/// > reset; azero off; nplc 10; dcv-source 1 ;  himux dcv-source ; azmux pcout; pc signal; gain 1;   trig;


nov 4.

  cal.
  cols = 3
  stderr(V) 0.67uV  (nplc10)


at 10nplc
> reset; azero on; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; buffer 30;  trig
countS  10002 2022507 1977610    712 4000001 (hi)  (hi -0.000,001,9V) (lo 0.000,004,6V, 0.000,004,8V) az meas -0.000,006,6V   mean(30) -0.0000074V, stddev(30) 0.67uV,
counts  10002 2022507 1977610    656 4000001 (lo)  (hi -0.000,001,9V) (lo 0.000,003,5V, 0.000,004,6V) az meas -0.000,005,9V   mean(30) -0.0000073V, stddev(30) 0.72uV,
counts  10002 2022507 1977610    720 4000001 (hi)  (hi -0.000,002,7V) (lo 0.000,003,5V, 0.000,004,6V) az meas -0.000,006,7V   mean(30) -0.0000073V, stddev(30) 0.72uV,
counts  10002 2022507 1977610    644 4000001 (lo)  (hi -0.000,002,7V) (lo 0.000,004,7V, 0.000,003,5V) az meas -0.000,006,8V   mean(30) -0.0000073V, stddev(30) 0.72uV,
counts  10002 2022507 1977610    725 4000001 (hi)  (hi -0.000,003,1V) (lo 0.000,004,7V, 0.000,003,5V) az meas -0.000,007,2V   mean(30) -0.0000073V, stddev(30) 0.72uV,

  have -7V. or so difference.

at 1nplc.
> reset; azero on; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002 202317 197812    695 400001 (lo)  (hi -0.000,002,7V) (lo 0.000,001,1V, 0.000,005,0V) az meas -0.000,005,8V   mean(30) -0.0000072V, stddev(30) 1.80uV,
counts  10002 202317 197812    701 400001 (hi)  (hi -0.000,004,7V) (lo 0.000,001,1V, 0.000,005,0V) az meas -0.000,007,7V   mean(30) -0.0000074V, stddev(30) 1.65uV,
counts  10002 202317 197812    692 400001 (lo)  (hi -0.000,004,7V) (lo 0.000,004,0V, 0.000,001,1V) az meas -0.000,007,2V   mean(30) -0.0000075V, stddev(30) 1.51uV,
counts  10002 202317 197812    699 400001 (hi)  (hi -0.000,002,7V) (lo 0.000,004,0V, 0.000,001,1V) az meas -0.000,005,3V   mean(30) -0.0000075V, stddev(30) 1.53uV,
counts  10002 202317 197812    691 400001 (lo)  (hi -0.000,002,7V) (lo 0.000,005,0V, 0.000,004,0V) az meas -0.000,007,2V   mean(30) -0.0000075V, stddev(30) 1.53uV,

  have -7V. or so difference.


------------

When sampling star-lo (from himux, and lomux (via R417=4.7k) there is about -4 to -5uV.

> reset; azero on; nplc 1; himux star-lo ; azmux star-lo ; gain 1; buffer 30;  trig
counts  10002 202317 197812    804 400001 (hi)  (hi -0.000,104,1V) (lo -0.000,098,3V, -0.000,100,2V) az meas -0.000,004,8V   mean(30) -0.0000045V, stddev(30) 2.03uV,
counts  10002 202317 197812    798 400001 (lo)  (hi -0.000,104,1V) (lo -0.000,098,3V, -0.000,098,3V) az meas -0.000,005,8V   mean(30) -0.0000045V, stddev(30) 1.98uV,
counts  10002 202317 197812    804 400001 (hi)  (hi -0.000,104,1V) (lo -0.000,098,3V, -0.000,098,3V) az meas -0.000,005,8V   mean(30) -0.0000045V, stddev(30) 1.99uV,
counts  10002 202317 197812    801 400001 (lo)  (hi -0.000,104,1V) (lo -0.000,101,2V, -0.000,098,3V) az meas -0.000,004,3V   mean(30) -0.0000045V, stddev(30) 1.99uV,
counts  10002 202317 197812    804 400001 (hi)  (hi -0.000,104,1V) (lo -0.000,101,2V, -0.000,098,3V) az meas -0.000,004,3V   mean(30) -0.0000044V, stddev(30) 1.98uV,

  As a separate point the ~= 100uV difference between star-lo and ref-lo, looks about right.
  since the lt1021/sense tap is lifted wrt the star-gnd, due to trace resistance and reference current.

    https://www.eeweb.com/tools/trace-resistance/.  0.4mm, 6", 1 Oz copper,  get 0.185R
    lt1021. ref current. around 1mA.
    V=IR =  0.001A * 0.185R = 185uV.
    It's the right order.


OK. sampling dcv-source works well.
>
reset; dcv-source 1 ; azero on; nplc 10; himux dcv-source ; azmux ref-lo ; gain 1; buffer 30;  trig

-
>  reset; dcv-source -10 ; azero on; nplc 1; himux dcv-source ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002 202317 197812    704 400001 (lo)  (hi -9.805,607,5V) (lo -0.000,007,6V, -0.000,006,6V) az meas -9.805,600,4V   mean(30) -9.8056006V, stddev(30) 2.55uV,
counts  10002 88995 311168     547 400001 (hi)  (hi -9.805,608,4V) (lo -0.000,007,6V, -0.000,006,6V) az meas -9.805,601,4V   mean(30) -9.8056007V, stddev(30) 2.55uV,
counts  10002 202317 197812    703 400001 (lo)  (hi -9.805,608,4V) (lo -0.000,006,6V, -0.000,007,6V) az meas -9.805,601,4V   mean(30) -9.8056007V, stddev(30) 2.55uV,
counts  10002 88995 311168     545 400001 (hi)  (hi -9.805,606,5V) (lo -0.000,006,6V, -0.000,007,6V) az meas -9.805,599,4V   mean(30) -9.8056006V, stddev(30) 2.55uV,
counts  10002 202317 197812    700 400001 (lo)  (hi -9.805,606,5V) (lo -0.000,003,7V, -0.000,006,6V) az meas -9.805,601,4V   mean(30) -9.8056006V, stddev(30) 2.54uV,
counts  10002 88995 311168     542 400001 (hi)  (hi -9.805,603,6V) (lo -0.000,003,7V, -0.000,006,6V) az meas -9.805,598,5V   mean(30) -9.8056005V, stddev(30) 2.57uV,


OK. negative source works too. with adjusted integrator range. NICE.
------------------

stderr(V) 0.66uV  (nplc10)

  //////////////////
another problem the lo increases at 1nplc versus 10nplc.
with -10V dc bias.

reset; dcv-source -10 ; azero on; nplc 10; himux dcv-source ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002  889117 3111017    432 4000001 (hi)  (hi -9.805,771,0V) (lo -0.000,002,2V, -0.000,001,0V) az meas -9.805,769,4V   mean(30) -9.8057689V, stddev(30) 1.17uV,
counts  10002 2022507 1977610    881 4000001 (lo)  (hi -9.805,771,0V) (lo -0.000,001,3V, -0.000,002,2V) az meas -9.805,769,3V   mean(30) -9.8057690V, stddev(30) 1.13uV,
counts  10002  889117 3111017    450 4000001 (hi)  (hi -9.805,772,8V) (lo -0.000,001,3V, -0.000,002,2V) az meas -9.805,771,1V   mean(30) -9.8057691V, stddev(30) 1.16uV,
counts  10002 2022507 1977610    889 4000001 (lo)  (hi -9.805,772,8V) (lo -0.000,002,1V, -0.000,001,3V) az meas -9.805,771,1V   mean(30) -9.8057692V, stddev(30) 1.15uV,
counts  10002  889117 3111017    448 4000001 (hi)  (hi -9.805,772,6V) (lo -0.000,002,1V, -0.000,001,3V) az meas -9.805,770,9V   mean(30) -9.8057693V, stddev(30) 1.18uV,
counts  10002 2022507 1977610    884 4000001 (lo)  (hi -9.805,772,6V) (lo -0.000,001,6V, -0.000,002,1V) az meas -9.805,770,8V   mean(30) -9.8057694V, stddev(30) 1.18uV,

reset; dcv-source -10 ; azero on; nplc 1; himux dcv-source ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002  202317  197812    719 400001 (lo)  (hi -9.805,790,9V) (lo -0.000,014,9V, -0.000,012,9V) az meas -9.805,777,0V   mean(30) -9.8057774V, stddev(30) 2.30uV,
counts  10002   88995  311168    764 400001 (hi)  (hi -9.805,792,8V) (lo -0.000,014,9V, -0.000,012,9V) az meas -9.805,778,9V   mean(30) -9.8057776V, stddev(30) 2.19uV,
counts  10002  202317  197812    712 400001 (lo)  (hi -9.805,792,8V) (lo -0.000,008,0V, -0.000,014,9V) az meas -9.805,781,4V   mean(30) -9.8057779V, stddev(30) 2.19uV,
counts  10002   88995  311168    759 400001 (hi)  (hi -9.805,787,9V) (lo -0.000,008,0V, -0.000,014,9V) az meas -9.805,776,5V   mean(30) -9.8057779V, stddev(30) 2.17uV,
counts  10002  202317  197812    716 400001 (lo)  (hi -9.805,787,9V) (lo -0.000,012,0V, -0.000,008,0V) az meas -9.805,777,9V   mean(30) -9.8057779V, stddev(30) 2.17uV,
counts  10002   88995  311168    760 400001 (hi)  (hi -9.805,788,9V) (lo -0.000,012,0V, -0.000,008,0V) az meas -9.805,778,9V   mean(30) -9.8057780V, stddev(30) 2.15uV,

about a from -1uV to -12uV. offset. for lo. not good.

with f+10V dc bias.

et; dcv-source 10 ; azero on; nplc 10; himux dcv-source ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002 3164482  835703    918 4000001 (hi)  (hi 9.879,711,8V) (lo -0.000,007,4V, -0.000,007,6V) az meas 9.879,719,3V   mean(30) 9.8797202V, stddev(30) 0.89uV,
counts  10002 2022507 1977610    947 4000001 (lo)  (hi 9.879,711,8V) (lo -0.000,007,7V, -0.000,007,4V) az meas 9.879,719,4V   mean(30) 9.8797202V, stddev(30) 0.90uV,
counts  10002 3164482  835703    901 4000001 (hi)  (hi 9.879,713,5V) (lo -0.000,007,7V, -0.000,007,4V) az meas 9.879,721,0V   mean(30) 9.8797202V, stddev(30) 0.91uV,
counts  10002 2022490 1977610    211 4000001 (lo)  (hi 9.879,713,5V) (lo -0.000,008,3V, -0.000,007,7V) az meas 9.879,721,5V   mean(30) 9.8797202V, stddev(30) 0.92uV,
counts  10002 3164482  835703    914 4000001 (hi)  (hi 9.879,712,2V) (lo -0.000,008,3V, -0.000,007,7V) az meas 9.879,720,2V   mean(30) 9.8797202V, stddev(30) 0.87uV,
counts  10002 2022490 1977610    205 4000001 (lo)  (hi 9.879,712,2V) (lo -0.000,007,7V, -0.000,008,3V) az meas 9.879,720,2V   mean(30) 9.8797201V, stddev(30) 0.85uV,

 reset; dcv-source 10 ; azero on; nplc 1; himux dcv-source ; azmux ref-lo ; gain 1; buffer 30;  trig
counts  10002  202317  197812    716 400001 (lo)  (hi 9.879,709,6V) (lo -0.000,012,0V, -0.000,013,9V) az meas 9.879,722,6V   mean(30) 9.8797211V, stddev(30) 1.32uV,
counts  10002  316540   83657    212 400001 (hi)  (hi 9.879,705,7V) (lo -0.000,012,0V, -0.000,013,9V) az meas 9.879,718,6V   mean(30) 9.8797210V, stddev(30) 1.38uV,
counts  10002  202317  197812    717 400001 (lo)  (hi 9.879,705,7V) (lo -0.000,012,9V, -0.000,012,0V) az meas 9.879,718,2V   mean(30) 9.8797210V, stddev(30) 1.47uV,
counts  10002  316557   83657    952 400001 (hi)  (hi 9.879,707,3V) (lo -0.000,012,9V, -0.000,012,0V) az meas 9.879,719,8V   mean(30) 9.8797209V, stddev(30) 1.49uV,
counts  10002  202317  197812    717 400001 (lo)  (hi 9.879,707,3V) (lo -0.000,012,9V, -0.000,012,9V) az meas 9.879,720,3V   mean(30) 9.8797209V, stddev(30) 1.49uV,

the lo is shifting....
maybe just needs amplifier divider compensation current - from amplifier.  or from adc hystersis..
about a from -6uV to -12uV. offset. for lo. not good.

EXTR - need to check its not . at 1nplc again. w

-----------------------------------------
***************
HOW TO determine - if the difference wehen sampling ref-lo from himux versus sampling ref-lo from lo-mux in az mode. - is a real voltage difference.
    - we just sample in non-az mode.   using pc-out.
***************
  - THIS SHOULD JUST ABOUT BE A TEST.
  - eg. take 5 samples of ref-lo via himux.   and 5 samples of ref-lo via lomux.  both in non-az mode.
  - that will reveal if its a temperature effect.
---------------------------------



Or need compensation. for the amplifier current.
Or need compensation for comparator.    it would be positive current.

if 1mA current,  on vref.  and copper can preturb 100uV.   then slight changes in ref-currents could  effect somewhow.
-----------------------------------

  EXTR - himux - and lomux areas are thermally isolated - because they are separate copper islands for guarding.
-------------------------



nov 5.

Wow. ok there really is around 7uV in it. between ref-lo from hi-mux versus lo-mux
by sampling in no-az. mode. so there are switch artiacts/ or settle timing issues.

sample ref-lo from azmux .
> reset; azero off; nplc 10; himux ref-lo; azmux ref-lo ; pc signal ;  gain 1;   trig
counts  10002 2022507 1977610    660 4000001 no-az meas meas 0.000,002,7Vf   mean(10) 0.0000035V, stddev(10) 0.45uV,
counts  10002 2022507 1977610    655 4000001 no-az meas meas 0.000,003,2Vf   mean(10) 0.0000034V, stddev(10) 0.42uV,
counts  10002 2022507 1977610    659 4000001 no-az meas meas 0.000,002,8Vf   mean(10) 0.0000034V, stddev(10) 0.47uV,
counts  10002 2022507 1977610    643 4000001 no-az meas meas 0.000,004,3Vf   mean(10) 0.0000035V, stddev(10) 0.56uV,
counts  10002 2022507 1977610    653 4000001 no-az meas meas 0.000,003,4Vf   mean(10) 0.0000035V, stddev(10) 0.56uV,
counts  10002 2022507 1977610    660 4000001 no-az meas meas 0.000,002,7Vf   mean(10) 0.0000034V, stddev(10) 0.62uV,

sample ref-lo from himux.
> reset; azero off; nplc 10; himux ref-lo; azmux pcout ; pc signal ;  gain 1;   trig
counts  10002 2022507 1977610    731 4000001 no-az meas meas -0.000,004,4Vf   mean(10) -0.0000039V, stddev(10) 0.55uV,
counts  10002 2022507 1977610    735 4000001 no-az meas meas -0.000,004,8Vf   mean(10) -0.0000040V, stddev(10) 0.59uV,
counts  10002 2022507 1977610    736 4000001 no-az meas meas -0.000,004,9Vf   mean(10) -0.0000041V, stddev(10) 0.57uV,
counts  10002 2022507 1977610    729 4000001 no-az meas meas -0.000,004,2Vf   mean(10) -0.0000042V, stddev(10) 0.57uV,



#endif



