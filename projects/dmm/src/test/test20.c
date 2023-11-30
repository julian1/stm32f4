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
counts  10002 2022507 1977610    660 4000001 no-az meas 0.000,002,7Vf   mean(10) 0.0000035V, stddev(10) 0.45uV,
counts  10002 2022507 1977610    655 4000001 no-az meas 0.000,003,2Vf   mean(10) 0.0000034V, stddev(10) 0.42uV,
counts  10002 2022507 1977610    659 4000001 no-az meas 0.000,002,8Vf   mean(10) 0.0000034V, stddev(10) 0.47uV,
counts  10002 2022507 1977610    643 4000001 no-az meas 0.000,004,3Vf   mean(10) 0.0000035V, stddev(10) 0.56uV,
counts  10002 2022507 1977610    653 4000001 no-az meas 0.000,003,4Vf   mean(10) 0.0000035V, stddev(10) 0.56uV,
counts  10002 2022507 1977610    660 4000001 no-az meas 0.000,002,7Vf   mean(10) 0.0000034V, stddev(10) 0.62uV,

sample ref-lo from himux.
> reset; azero off; nplc 10; himux ref-lo; azmux pcout ; pc signal ;  gain 1;   trig
counts  10002 2022507 1977610    731 4000001 no-az meas -0.000,004,4Vf   mean(10) -0.0000039V, stddev(10) 0.55uV,
counts  10002 2022507 1977610    735 4000001 no-az meas -0.000,004,8Vf   mean(10) -0.0000040V, stddev(10) 0.59uV,
counts  10002 2022507 1977610    736 4000001 no-az meas -0.000,004,9Vf   mean(10) -0.0000041V, stddev(10) 0.57uV,
counts  10002 2022507 1977610    729 4000001 no-az meas -0.000,004,2Vf   mean(10) -0.0000042V, stddev(10) 0.57uV,




nov 6
  added ltc1000,
  cols 3. stderr(V) 0.67uV  (nplc10)

  with 100 sample buffer.
  noise seems the same or higher,

  10 nplc.
  > reset; azero off; nplc 10; himux ref-lo; azmux pcout ; pc signal ;  gain 1; buffer 100;  trig
  counts  10002 2022507 1977661    536 4000001 no-az s=76 rows=100 meas -0.000,004,9V   f mean(100) -0.0000043V, stddev(100) 0.43uV,

  1nplc.
  > reset; azero off; nplc 1; himux ref-lo; azmux pcout ; pc signal ;  gain 1; buffer 100;  trig
  counts  10002  202317  197812    907 400001 no-az s=38 rows=100 meas -0.000,007,2V   f mean(100) -0.0000096V, stddev(100) 1.48uV,

  az 10nplc
  > reset;  ; azero on; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; buffer 100;  trig
  counts  10002 2022490 1977644    503 4000001 az  (lo)  (hi -0.000,009,2V) (lo 0.000,003,0V, 0.000,001,8V) meas -0.000,011,6V   f mean(100) -0.0000109V, stddev(100) 0.51uV,

  at 1nplc
  reset;  ; azero on; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; buffer 100;  trig
  counts  10002  202317  197812    918 400001 az  (hi)  (hi -0.000,016,0V) (lo -0.000,003,4V, -0.000,004,3V) meas -0.000,012,2V   f mean(100) -0.0000113V, stddev(100) 1.75uV,


  measuring temperature
  reset; dcv-source temp ; azero on; nplc 10; himux dcv-source ; azmux ref-lo ; gain 1; buffer 30;  trig

  reset; dcv-source temp ; azero off; nplc 10; himux dcv-source ; azmux pcout; pc signal ; gain 1; buffer 10;  trig

  ---------------------------
  --------
  reset; azero off; nplc 10; himux ref-lo; azmux pcout ; pc signal ;  gain 1; buffer 100;  trig;

  No lid. two runs. 100 obs.
    counts  10002 2022507 1977661    617 4000001 no-az meas -0.000,004,9V   f mean(100) -0.0000051V, stddev(100) 0.54uV,
    counts  10002 2022507 1977661    610 4000001 no-az meas -0.000,004,2V   f mean(100) -0.0000043V, stddev(100) 0.61uV,

  With lid.
  counts  10002 2022507 1977661    620 4000001 no-az meas -0.000,005,2V   f mean(100) -0.0000045V, stddev(100) 0.42uV,


solder in 100k zfoil for bias resistor.
  after soldering,   and cleaning.
  stderr(V) 24uV  (nplc10)
  stderr(V) 7.98uV  (nplc10)
  stderr(V) 3.25uV  (nplc10)
  stderr(V) 0.79uV  (nplc10)
  stderr(V) 0.67uV  (nplc10)
  stderr(V) 0.87uV  (nplc10)

    So it plays a part.

  no lid.  0.61uV.  10nplc.  n=100,20 sec.
  with lid.  0.42V.  10nplc.  n=100,20 sec.

  Ok. so there's not much difference.

  To rule out some potential issues - I swapped the ref over, changed the rundown bias resistor from inexpensive thin-film to zfoil to rule out thermal variantion
  Also added gain to the slope amplifier,  from opa140 and 10k/10k to opa140 and 2.15k/21.5k (2k loading on integrator output is probably too high).
  In the past I tried perturbing the slope-amp and gain with different bjt op-amps - ne5534, op27, op37, lt1358. but without seeing much difference.
  the reset resistor is 3.74k. not 20k. as indicated on the schematic (also a load on the integrator op).

  The biggest difference is seen by adding tin lid for shielding. 0.6uV -> 0.4uV.  10nplc. az off.

  Two things in my mind are  - populate the RC between the adc mux and integrator input.
  It is a bit spikey when probed with a scope. but I never got around to populating them.
  Also to change the 2k+2k+mux rds(on). as the input resistance.
  But maybe white-noise/flicker noise  would be evident from a frequency/noise like plot.




  stderr(V) 0.85uV  (nplc10)
  adding the slope gain makes no difference or worsens the result 0.4uV to 0.5uV.
  cols = 3, stderr(V) 0.67uV  (nplc10)
  -----
  with lid.

  stderr(V) 0.51uV  (nplc10)    <- OK, this is slightly better.

  reset; dcv-source temp ; azero on; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; buffer 200;  trig
  counts  10002  202317  197812    681 400001 az  (hi)  (hi -0.000,013,7V) (lo -0.000,000,9V, 0.000,000,0V) meas -0.000,013,3V   f mean(200) -0.0000108V, stddev(200) 1.48uV,



nov 8.

  stddev. seems worse. after making changes.

  stderr(V) 1.65uV  (nplc10)   it's worse.
  stderr(V) 0.93uV  (nplc10)


  produced allan deviation plot. in timelab.

  reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1;  trig


nov 10

  baseline

    AZ
    > reset; azero on; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; buffer 100;  trig
    counts  10002 2022507 1977593    822 4000001 az (lo  ref-lo) (hi -0.000,007,4V) (lo 0.000,004,4V, 0.000,003,1V) meas -0.000,011,2V mean(100) -0.0000114V, stddev(100) 0.48uV, offset = 11uV.
    counts  10002 2022507 1977593    950 4000001 az (hi ref-lo)  (hi -0.000,008,2V) (lo 0.000,004,2V, 0.000,003,8V) meas -0.000,012,2V mean(100) -0.0000114V, stddev(100) 0.48uV,

    >  reset; azero on; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; buffer 100;  trig
    counts  10002  202317  197812    636 400001 az (lo  ref-lo) (hi -0.000,016,0V) (lo -0.000,003,2V, -0.000,004,2V) meas -0.000,012,3V mean(100) -0.0000107V, stddev(100) 1.35uV,  offset = 12uV.
    counts  10002  202317  197812    633 400001 az (lo  ref-lo) (hi -0.000,014,0V) (lo -0.000,000,2V, -0.000,001,2V) meas -0.000,013,3V mean(100) -0.0000112V, stddev(100) 1.39uV

    no AZ
    > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
    counts  10002 2022507 1977593    823 4000001 no-az (ref-lo) meas 0.000,004,3V mean(100) 0.0000036V, stddev(100) 0.41uV,

    > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
    counts  10002  202317  197812    635 400001 no-az (ref-lo) meas -0.000,002,2V mean(100) -0.0000022V, stddev(100) 1.27uV,
    counts  10002  202317  197812    636 400001 no-az (ref-lo) meas -0.000,003,2V mean(100) -0.0000027V, stddev(100) 1.04uV,

  --------------------
  add RC filtering of ref. 1k/ 1u. PP  cap.  tin-lid shielding. could not cover reference properly.
    ----
    stderr(V) 0.65uV  (nplc10)


    AZ.
    reset; azero on; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; buffer 100;  trig
    counts  10002 2022507 1977593    725 4000001 az (lo  ref-lo) (hi -0.000,009,0V) (lo 0.000,001,8V, 0.000,001,6V) meas -0.000,010,6V mean(100) -0.0000105V, stddev(100) 0.42uV,   offset = 10uV.

    reset; azero on; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; buffer 100;  trig
    counts  10002  202317  197812    622 400001 az (lo  ref-lo) (hi -0.000,013,9V) (lo -0.000,001,1V, -0.000,003,0V) meas -0.000,011,8V mean(100) -0.0000109V, stddev(100) 1.35uV,  offset = 11uV.

    no AZ.
    >  reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
    counts  10002 2022507 1977593    710 4000001 no-az (ref-lo) meas 0.000,003,2V mean(100) 0.0000030V, stddev(100) 0.41uV,k

    > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
    counts  10002  202317  197812    620 400001 no-az (ref-lo) meas -0.000,006,6V mean(100) -0.0000058V, stddev(100) 1.23uV,

  -------------------
  add RC filtering of ref - use 1k/1u x7r.  - but place tin-lid shielding.

    stderr(V) 0.56uV  (nplc10)            <- this is pretty good.
    stderr(V) 0.74uV  (nplc10)
    stderr(V) 0.65uV  (nplc10

    NO AZ.
    > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
    counts  10002 2022507 1977593    840 4000001 no-az (ref-lo) meas -0.000,000,8V mean(100) -0.0000005V, stddev(100) 0.38uV       a little better.
    counts  10002 2022507 1977593    863 4000001 no-az (ref-lo) meas -0.000,003,1V mean(100) -0.0000027V, stddev(100) 0.41uV,     nope. the same.

    > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
    counts  10002  202317  197812    639 400001 no-az (ref-lo) meas -0.000,019,7V mean(100) -0.0000198V, stddev(100) 1.28uV

    no change.

  -----------------

  add 220p/ 100R. at start of integrator.
  solder and clean. around sensitive parts. may need timie to settle.

    from 20uV.
    stderr(V) 0.55uV  (nplc10)  after ten minutes.
    stderr(V) 0.55uV  (nplc10)   <- this is a bit better.
    stderr(V) 0.57uV  (nplc10)

  > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
  counts  10002 2022507 1977627    621 4000001 no-az (ref-lo) meas 0.000,000,9V mean(100) 0.0000010V, stddev(100) 0.38uV,


  stderr(V) 0.43uV  (nplc10)   <- wow.


nov 11

  After refactoring. to use clk count down.
  > cal
  stderr(V) 0.62uV  (nplc10)
  stderr(V) 0.40uV  (nplc10)
  stderr(V) 0.73uV  (nplc10)
  stderr(V) 0.61uV  (nplc10)


  adc_modulation_04.v after simplifying the fast rundown.

  stderr(V) 0.55uV  (nplc10)
  stderr(V) 0.69uV

  10nplc. the same.

  > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
  counts  10002  202299  197802    411 400001 no-az (ref-lo) meas -0.000,014,6V mean(100) -0.0000144V, stddev(100) 1.37uV,

  > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100 ;  trig
  counts  10002 2022489 1977611    405 4000001 no-az (ref-lo) meas 0.000,001,1V mean(100) 0.0000013V, stddev(100) 0.37uV,



nov 12

  chaged from 1.5k/475R. to 5.9k/1k compound divideer. 10mins after soldering.

  stderr(V) 0.61uV  (nplc10)

  >  reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100;  trig
  clk counts  202299  197802    411 400001 no-az (ref-lo) meas 0.000,015,6V mean(100) 0.0000179V, stddev(100) 1.23uV,

  no. change and the frequency is still the same. - hang on must check the voltage range.

  change RN902 from 10k. to 1k.

    res       0.176uV  digits 7.75   (nplc10)
    but no improvement.

  > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100;  trig
  clk counts  204155  195959    228 400001 no-az (ref-lo) meas -0.000,005,3V mean(100) -0.0000046V, stddev(100) 1.20uV,


nov 13


  - powered up using regulators on +-18V. and bridge-rectifiers.  but no improvement in noise.

    stderr(V) 0.81uV  (nplc10)
    stderr(V) 0.86uV  (nplc10)
    stderr(V) 0.86uV  (nplc10)

    > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
    clk counts 2040993 1959131    242 4000001 no-az (ref-lo) meas 0.000,016,4V mean(10) 0.0000170V, stddev(10) 0.54uV,
    - need to try the 34401a transformer.

    -----
    - soldered all lt5400 centre-pads.
    - revert lt5400 1k.  for RN902 -> 10k
    - revert 21k/2k.   -> 10k/10k slope
    - revert 6k/1k -> 1.54k / 475R. for compound.

    ------

    - everything seems works.  within 20mins of soldering.
    stderr(V) 1.16uV  (nplc10)
    stderr(V) 0.78uV  (nplc10)    30mins . hmmmm
    stderr(V) 0.62uV  (nplc10)
    stderr(V) 0.81uV  (nplc10)    hour later.
    stderr(V) 0.77uV  (nplc10)

  change first integrator to opa202.
    stderr(V) 1.80uV  (nplc10)    five mins after soldering.
    stderr(V) 0.81uV  (nplc10)    about 10 mins.   quite good.
    stderr(V) 0.75uV  (nplc10)   couple hours. no improvement.

    workman usig power-tools outside. kk

  - change back opa140.

  stderr(V) 0.63uV  (nplc10)
  stderr(V) 0.61uV  (nplc10)

      better.


  - chage to 34401a transformer
      not very good 60pF. coupling capacitance. and no block filter.
      but it works.

  stderr(V) 0.85uV  (nplc10)
  stderr(V) 0.58uV  (nplc10)
  stderr(V) 0.67uV  (nplc10)

  no difference.
    this was uncovered.

  1nplc.  clk counts  202296  197855    347 400001 no-az (ref-lo) meas 0.000,023,3V mean(100) 0.0000229V, stddev(100) 1.65uV,
  10nplc  clk counts 2022225 1977899    347 4000001 no-az (ref-lo) meas 0.000,002,3V mean(100) 0.0000023V, stddev(100) 0.55uV

  covered. tin lid
    stderr(V) 0.74uV  (nplc10)
    stderr(V) 0.67uV  (nplc10)

  the same.

    stderr(V) 0.45uV  (nplc10) <- rather lo.
    stderr(V) 0.54uV  (nplc10
    stderr(V) 0.54uV  (nplc10)

  -----



nov 14.

    stderr(V) 0.67uV  (nplc10)a     no lid.
    stderr(V) 0.65uV  (nplc10)
    stderr(V) 0.64uV  (nplc10)      lid.


    - removed ferrite bead in front of oscillator.  changed to 15R.

    stderr(V) 0.63uV  (nplc10)   no lid.
    stderr(V) 0.53uV  (nplc10)   no lid.   best without a lid.
    stderr(V) 0.56uV  (nplc10)      no lid.

    -  OK. looks a bit better.
  clk counts 2022225 1977899    204 4000001 no-az (ref-lo) meas 0.000,001,1V mean(10) 0.0000011V, stddev(10) 0.32uV,

  stderr(V) 0.62uV  (nplc10)    with lid , but sits a bit higher.
  stderr(V) 0.61uV  (nplc10)


  Fixed the AZ cycling. which was getting wrongly triggered.


  The 8uV. offset has gone.  perhaps due to removing the ferite bead... Very nice.


   hi reset; azero on; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; buffer 10;  trig
 lo clk counts 2022224 1977899    225 4000001 az (lo  ref-lo) (hi 0.000,000,5V) (lo -0.000,001,1V, 0.000,002,1V) meas -0.000,000,0V mean(10) -0.0000004V, stddev(10) 0.73uV,
 hi clk counts 2022224 1977899    209 4000001 az (hi ref-lo)  (hi 0.000,000,4V) (lo -0.000,001,1V, 0.000,002,1V) meas -0.000,000,1V mean(10) -0.0000005V, stddev(10) 0.68uV,
 lo clk counts 2022224 1977899    229 4000001 az (lo  ref-lo) (hi 0.000,000,4V) (lo -0.000,001,4V, -0.000,001,1V) meas 0.000,001,7V mean(10) -0.0000003V, stddev(10) 0.95uV,
 hi clk counts 2022224 1977899    231 4000001 az (hi ref-lo)  (hi -0.000,001,6V) (lo -0.000,001,4V, -0.000,001,1V) meas -0.000,000,4V mean(10) -0.0000003V, stddev(10) 0.94uV,

nov 15.

  no change from yesterday.  - but after  second triggering.

  stderr(V) 0.60uV  (nplc10)    no lid.
  stderr(V) 0.58uV  (nplc10)  with lid
  stderr(V) 0.52uV  (nplc10)  with lid.

  noise seems a little higher.
  may want to revert to a little slope-gain.

  clk counts  202293  197855    206 400001 no-az (ref-lo) meas -0.000,003,6V mean(100) -0.0000057V, stddev(100) 1.67uV,  with lid.


  after soldering synchronizer. 74lv175.  replacing 74hc175

    stderr(V) 0.75uV  (nplc10) with lid.
    stderr(V) 0.77uV  (nplc10) with lid.  hmmmm its worse.

    no improvement.
      but does affect the counts/ final resolution rundown. a little . which is interesting.
      res       0.075uV  digits 8.12   (nplc10)

      now changed back.
      stderr(V) 0.69uV  (nplc10)
      res       0.097uV  digits 8.01   (nplc10)


  - change lt1016. to  tl3016. and change ferrite beads to 10R.
    - cooler, and but maybe more twitchy/  and need more hysteriis..

    stderr(V) 0.71uV  (nplc10)
    stderr(V) 0.74uV  (nplc10) res       0.075uV  digits 8.13   (nplc10)

    stderr(V) 0.53uV  (nplc10)      with lid. move phone away.
    stderr(V) 0.55uV  (nplc10)      same

    stderr(V) 0.64uV  (nplc10)      with lid.   phone next to pcb.
    stderr(V) 0.46uV  (nplc10)      same    lowest ever?
    stderr(V) 0.46uV  (nplc10)      same.

    stderr(V) 0.71uV  (nplc10)      move phone away???  bizarre.
    stderr(V) 0.49uV  (nplc10)      same.



  - BUT WE CAN SEE ALL THIS On the Scope. it's the accumulation.



  replace lt5400 with MORN - - 5 mins after soldering.

    stderr(V) 0.78uV  (nplc10)
    res       0.112uV  digits 7.95   (nplc10)

    stderr(V) 0.52uV  (nplc10)   with lid.
    stderr(V) 0.63uV  (nplc10)
    stderr(V) 0.53uV  (nplc10)

    noise looks the same.
    we have to fix the comparator triggering.  more slope. more hysterysis.

    ,
    clk counts 2022896 1977185    217 4000001 no-az (ref-lo) meas 0.000,000,7V mean(100) 0.0000007V, stddev(100) 0.39uV,

    stderr(V) 0.45uV  (nplc10 )    about 20 mins after soldering. this is probably the best. we have seen. so it is a small improvement.


  - need to let it settle overnight.  then look at latching the comparator.


nov 16.

  no change.

    stderr(V) 0.70uV  (nplc10)
    stderr(V) 0.64uV  (nplc10)

    clk counts 2022891 1977190    198 4000001 no-az (ref-lo) meas 0.000,000,0V mean(100) -0.0000004V, stddev(100) 0.40uV,
  --------------------
  changed hysteris to 50x.  does not seem to have made rundown time much worse.
    - eliminated oscillation at rundown.
    - and reduced a lot at start.

    stderr(V) 0.69uV  (nplc10)
    stderr(V) 0.55uV  (nplc10)


    wow. noise is lower.
    wtf.  it's higher for 1nplc.

    > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
    clk counts 2022892 1977189    275 4000001 no-az (ref-lo) meas -0.000,001,1V mean(10) -0.0000011V, stddev(10) 0.17uV,

    noise is higher.  for 1nplc - because getting o

    AND NOISE IS BETTER. for 1nplc.
    EXTR. but it's obscure. because the main counts jump

    > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
    clk counts  202372  197795    312 400001 no-az (ref-lo) meas 0.000,014,5V mean(10) 0.0000152V, stddev(10) 0.47uV,
    clk counts  202372  197795    312 400001 no-az (ref-lo) meas 0.000,014,5V mean(10) 0.0000151V, stddev(10) 0.52uV,


    - after get 10 items.
    > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
    clk counts 2022893 1977191    290 4000001 no-az (ref-lo) meas -0.000,006,7V mean(10) -0.0000068V, stddev(10) 0.22uV,
    clk counts 2022893 1977191    287 4000001 no-az (ref-lo) meas -0.000,006,5V mean(10) -0.0000068V, stddev(10) 0.23uV

    WOW.


    > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
    clk counts  202371  197795    275 400001 no-az (ref-lo) meas -0.000,003,0V mean(20) -0.0000024V, stddev(20) 0.65uV,
    clk counts  202371  197795    275 400001 no-az (ref-lo) meas -0.000,003,0V mean(20) -0.0000020V, stddev(20) 0.83uV,


    EXTR - doesn't have lid touching gnd post.

  - changed slope amp  to 3.74k / 25k.

      seemed to have mostly fixed oscill at start. although not perfect.

    stderr(V) 0.71uV  (nplc10)
    res       0.081uV  digits 8.09   (nplc10)

    yep.
    > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
    clk counts 2022895 1977191    309 4000001 no-az (ref-lo) meas 0.000,006,2V mean(10) 0.0000068V, stddev(10) 0.26uV,
    clk counts 2022894 1977190    287 4000001 no-az (ref-lo) meas 0.000,008,1V mean(10) 0.0000078V, stddev(10) 0.23uV,

    > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
    clk counts  202373  197795    297 400001 no-az (ref-lo) meas 0.000,045,3V mean(10) 0.0000447V, stddev(10) 0.83uV,
    clk counts  202376  197795    295 400001 no-az (ref-lo) meas 0.000,177,0V mean(10) 0.0001773V, stddev(10) 0.66uV,
    clk counts  202373  197795    297 400001 no-az (ref-lo) meas 0.000,045,3V mean(100) 0.0000456V, stddev(100) 0.77uV,   100 samples.

    stderr(V) 0.57uV  (nplc10)   20mins after soldering.

    lol.
    seems worse now at least for  1nplc
    clk counts  202373  197795    307 400001 no-az (ref-lo) meas 0.000,011,8V mean(10) 0.0000099V, stddev(10) 1.53uV,

    10nplc is ok.
    clk counts 2022895 1977193    298 4000001 no-az (ref-lo) meas 0.000,008,4V mean(10) 0.0000084V, stddev(10) 0.25uV,
    clk counts 2022894 1977192    290 4000001 no-az (ref-lo) meas 0.000,001,1V mean(10) 0.0000010V, stddev(10) 0.25uV,      yes. it

    now higher????
    clk counts 2022894 1977192    276 4000001 no-az (ref-lo) meas 0.000,010,6V mean(10) 0.0000102V, stddev(10) 0.37uV,

    - perhaps we have rounding issue.






  - changed slope amp to fast ad847 and 10x.  similar to modern 3458a.
      - not much difference in noise. but still good to match 3458a.

      - AND - and it has stopped . the comparator output oscillation at the start.
      - GOOD.



  - try increasing slope. to reduce quantization?
      matters for 1nplc not so much 10nplc.
      changed  from 100k to 150k.

      stderr(V) 0.60uV  (nplc10)
      res       0.074uV  digits 8.13   (nplc10)     <- increased.  because of shallower slope.
      stderr(V) 0.57uV  (nplc10)

    OK. the noise - at least with scope leads is clearly in the o

  ----
  GAhhh. we fucked something. the scope probe dragged across pcb.

    ok. can control the integrator, by controlling the direct bits.
    we need to see the comparator output.

    - comparator isn't reflecting the integrator.
    - perhaps it's the latch.
    - integrator and slope amp look ok.
    - looks like comparator is fried. or else theo
    - replaced comparator and looks like its working again.
  -------------

  OK. we - can see the horizontal movement on scope. issue with noise of integrator - it's either resistors. or op-amps.


  change the speed/freq. of integration.
      - lower BW -> lower noise.
      - and will reduce influence of jitter.
      - and will reduce reversals/ and power supply effects - for no RC on




nov 17
  leave 24 hours. to settle.

  10nplc around 0.22uV.
  > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
  clk counts 2015650 1984433    357 4000001 no-az (ref-lo) meas 0.000,008,9V mean(10) 0.0000092V, stddev(10) 0.21uV,
  clk counts 2015650 1984430    354 4000001 no-az (ref-lo) meas 0.000,022,4V mean(10) 0.0000224V, stddev(10) 0.22uV,    <- low noise. but bad cal?

  1nplc around 0.60uV.
  > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100;  trig
  clk counts  201638  198509    339 400001 no-az (ref-lo) meas 0.000,158,0V mean(100) 0.0001570V, stddev(100) 0.59uV,
  clk counts  201638  198509    343 400001 no-az (ref-lo) meas 0.000,156,4V mean(100) 0.0001568V, stddev(100) 0.55uV,
  clk counts  201638  198509    339 400001 no-az (ref-lo) meas 0.000,158,0V mean(100) 0.0001575V, stddev(100) 0.59uV      <- notice offset. (bad cal?)
  The adc integrator is currently using 330p.  84kHz  (by +ref current switch up count).
  A lower integration freq would likely reduce noise more according to sqrt( BW ). and also if switch jitter, and HF transitions are relevant.



  ---------
I've made some changes to improve adc noise a bit.

changes creating obvious improvement -
  - swap lt5400 to morn resistors that I had at hand. these look superior from Castorp's published research, reduced 1.3uV  to 1uV RMS 1nplc.

  - nov 18. lowering freq. 330p/84kHz to 1.5n/ 19Khz. reduced 1uV to about 0.7uV RMS 1nplc.
    This improvement is less than the expected ( Math.sqrt( 84kHz ) / Math.sqrt( 19kHz) = 2.1x ), if integrator op-amp noise was dominant.


other changes, with less noticeable influence

  - swap synchronizer 74hc175 to lv175, not sure how I overlooked this.
  - swap comparator from lt1016 to tl3016, mostly to reduce current and heat. lt1016, is horrible and the to-92 L7805/7905 regulators are hot and can barely be touched.
  - shallower rundown, less quantitization, although past tests suggest this may affect INL a little due to DA.

  - also increased slope-amp gain (to reduce slope-amp BW - and gave more hysteresis for the comparator, to ensure no output glitching/meta-stablility issues.
      (control over the comparator latch to prevent glitching, is not effective if the control cannot first sample a clean value on the clock edge).
      this seems to be more an issue for tl3016 v lt1016.
      although none of these changes appeared to influence measurement noise.

  - I couldn't see any difference, from adding LP filtering of the ref for HF noise with LC 1k/1u PP or x7r,
      Perhaps a series RC bypass to ground would be better - similar to the 1u/5R of the adr1399 reference circuit?
      - the ref sits right next to the adc current source ladder which is good.
      but the ref-lo and ref-hi traces cross the adc circuitry to reach the input muxes, so this could be an issue.
      It's not possible to LC filter the references here, so this has to be addressed with an improved layout.

  - using 34401a transformer to power the board, instead of a bench supply.

- I believe it's possible to see the adc noise on a scope.
    Hooking up scope/digital probes does ground reference the circuit, and introduces additional EMI, which increases measurement stddev 2x to 3x.
    But it is still possible to visually correlate the waveform variance and magnitude, which is mostly associated with runup (horizontal displacements of the waveform at the end of runup),
    rather than anything around rundown/comparator sampling.

- it should be possible to do a kind of rundown re-sample/multi-sample. So after final rundown, the output could be steered to perform a second rundown cycle
    And ths would improve rundown sampling noise.
    But given what can be seen on the scope, I doubt that it will help.
    Also switching to an ADC to multi-sample the rundown output would be more effective and simpler.



  ---------


  stderr(V) 0.41uV  (nplc10)      <- wow.   lowest ever.
  stderr(V) 0.49uV  (nplc10)
  stderr(V) 0.43uV  (nplc10)

  -----------

  Fucking hell.  now it's higher again?????

    After doing a calibration.

      > reset;  azero off; nplc 1; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 100;  trig
      clk counts  201638  198509    331 400001 no-az (ref-lo) meas 0.000,000,2V mean(100) -0.0000004V, stddev(100) 1.08uV,    <- but no offset issue. weird.

      > reset;  azero off; nplc 10; himux ref-lo ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig
      clk counts 2015661 1984427    356 4000001 no-az (ref-lo) meas 0.000,004,3V mean(10) 0.0000044V, stddev(10) 0.42uV,

      How. issue with the calibration constants?
      Absolutely no other change. have not even touched the board....

      Is there an issue with calibration and rundown slope... variation.

      stderr(V) 0.45uV  (nplc10)
      res       0.071uV  digits 8.15   (nplc10)

      1nplc
      clk counts  201638  198509    326 400001 no-az (ref-lo) meas -0.000,007,4V mean(100) -0.0000087V, stddev(100) 0.97uV,

      10nplc
      clk counts 2015662 1984427    358 4000001 no-az (ref-lo) meas 0.000,003,5V mean(10) 0.0000034V, stddev(10) 0.26uV


      10nplc. is not too far off. 3458a. noise.
      --
      we need the ability to save the cal.

    -----

    what do we do?
      - leave it and try again later?
      - try larger cap.
      - and slower counts.


    5.1nV * Math.sqrt(84000)
      = 1.47uV.

    5.1 is a datasheet. spec.
    But does the compound divider reduce.

  Try lt1028 as second stage op.

      stderr(V) 16.93uV  (nplc10),   res  0.104uV  digits 7.98   (nplc1
      cannot get good cal.

      OK. doesn't work. - integrator is current device. - so op-amp must have high imput impedance.
      may

      3458a uses jfet in front of bjt.

    ---------


    try 1.5n cap.
      without chaging fix and var. it all works. noise is up because have probes attached.

      stderr(V) 1.62uV  (nplc10)
      res       0.086uV  digits 8.07   (nplc10)

    1500/330 = 4.5x.
      increased fix,var counts by 4.5x also.
      freq. 19kHz.

      Math.sqrt( 84 ) / Math.sqrt( 19) = 2.1 x better.

    -------

    1nplc
    clk counts  201884  198691    381 400001 no-az (ref-lo) meas -0.000,022,9V mean(100) -0.0000230V, stddev(100) 0.76uV,

    10nplc
    clk counts 2016138 1984299    431 4000001 no-az (ref-lo) meas 0.000,001,9V mean(20) 0.0000023V, stddev(20) 0.27uV,

    stderr(V) 0.42uV  (nplc10)    <- very good.  res       0.070uV  digits 8.15   (nplc10)
    stderr(V) 0.46uV  (nplc10) res       0.069uV  digits 8.16   (nplc10)
    ---

    10nplc.
    clk counts 2016138 1984299    406 4000001 no-az (ref-lo) meas 0.000,001,7V mean(10) 0.0000015V, stddev(10) 0.21uV,

    1nplc
    clk counts  201884  198691    380 400001 no-az (ref-lo) meas 0.000,003,1V mean(100) 0.0000035V, stddev(100) 0.69uV,


    https://xdevs.mymsk.cloud/datashort/_done/DrFrank_HP3458A.txt     0.21uV.
    https://xdevs.mymsk.cloud/datashort/_done/radioFlash_34465A.txt   0.42uV. pp. 10nplc.

    https://xdevs.mymsk.cloud/datashort/_done/OldNeurons_34461A.txt  2.56uV.
    https://xdevs.mymsk.cloud/datashort/_done/HighVoltage_34461A.txt  |    0.26 uV. ?


nov 18
  change compound divider. from 1.5k/475R to 10k/1k.  same as K.DVM.

    -MUST check range on scope.
    - compound divider does *not* change range.

    stderr(V) 0.90uV  (nplc10)
    res       0.072uV  digits 8.14   (nplc10)

    10 mins after soldering.
    1nplc. clk counts  201885  198691    438 400001 no-az (ref-lo) meas -0.000,032,3V mean(100) -0.0000322V, stddev(100) 0.78uV,
           clk counts  201885  198691    434 400001 no-az (ref-lo) meas -0.000,029,4V mean(100) -0.0000289V, stddev(100) 0.84uV,

    seems slightly worse.
    maybe give time to settle.

    stderr(V) 0.47uV  (nplc10)
    res       0.067uV  digits 8.17   (nplc10)

    maybe 5k/500R. would be better.

    clk counts  201885  198691    434 400001 no-az (ref-lo) meas -0.000,010,3V mean(100) -0.0000097V, stddev(100) 0.83uV,



nov 19.
  no changes, since yesterday. settling board.
    stderr(V) 0.45uV  (nplc10) res       0.061uV  digits 8.22   (nplc10)
    stderr(V) 0.54uV  (nplc10)

    clk counts  201884  198691    395 400001 no-az (ref-lo) meas -0.000,002,1V mean(100) -0.0000023V, stddev(100) 0.73uV,   Good.
    clk counts  201884  198691    394 400001 no-az (ref-lo) meas -0.000,001,5V mean(100) -0.0000018V, stddev(100) 0.65uV,
    clk counts  201884  198691    393 400001 no-az (ref-lo) meas -0.000,000,8V mean(100) -0.0000017V, stddev(100) 0.70uV
    clk counts  201884  198691    393 400001 no-az (ref-lo) meas -0.000,000,8V mean(100) -0.0000003V, stddev(100) 0.67uV,


    Yes. we have 0.7uV.  Good.




  try low-jitter cmos xtal oscillator .

    4  minutes. cannot get a clean calibration. because of different counts. odd.

    stderr(V) 798951.20uV  (nplc10)
    res       364.477uV  digits 4.44   (nplc10)

    jitter sems lower on scope. but cannot. get a cal. odd.

    - perhaps edge is not good enough, without schmitt, and we get double triggering.


    - OK. changed resisotr from 50R to 10R. and it's improved. and we can get a cal. but some counts look off by one.

    stderr(V) 1.39uV  (nplc10) res       0.044uV  digits 8.36   (nplc10)

    noise appears the same.
    clk counts  201884  198691    386 400001 no-az (ref-lo) meas 0.000,073,7V mean(100) 0.0000742V, stddev(100) 0.74uV,

    no. noise is a lot worse,
    clk counts  201884  198691    385 400001 no-az (ref-lo) meas -0.000,035,7V mean(100) -0.0000354V, stddev(100) 1.26uV,

    SO. the resistor is really important.

    stderr(V) 0.84uV  (nplc10) res       0.070uV  digits 8.15   (nplc10)
    stderr(V) 0.47uV  (nplc10) res       0.068uV  digits 8.17   (nplc10)
    stderr(V) 0.46uV  (nplc10)

    clk counts  201884  198691    385 400001 no-az (ref-lo) meas -0.000,012,7V mean(100) -0.0000143V, stddev(100) 0.89uV,
    clk counts  201884  198691    389 400001 no-az (ref-lo) meas -0.000,004,5V mean(100) -0.0000040V, stddev(100) 0.84uV,

    Ok. think we want to revert. back to old xtal and 51R.



nov 20

  no changes since yesterday,

      now it's worse.... fuck.
      clk counts  201885  198691    433 400001 no-az (ref-lo) meas -0.000,015,2V mean(100) -0.0000161V, stddev(100) 0.94uV,

      issue may be fpga was connected.

      clk counts  201885  198691    437 400001 no-az (ref-lo) meas -0.000,017,9V mean(100) -0.0000177V, stddev(100) 0.93uV,
      clk counts  201885  198691    437 400001 no-az (ref-lo) meas -0.000,017,9V mean(100) -0.0000175V, stddev(100) 0.93uV,


      stderr(V) 0.44uV  (nplc10)                      <- good.
      res       0.068uV  digits 8.17   (nplc10)

      clk counts  201885  198691    439 400001 no-az (ref-lo) meas -0.000,015,2V mean(100) -0.0000145V, stddev(100) 1.02uV,
      clk counts  201885  198691    439 400001 no-az (ref-lo) meas -0.000,015,2V mean(100) -0.0000143V, stddev(100) 0.91uV,

      still bad.

  OK,
  moving the laptop away and the usb hub and wires and usb cord - improves.
      clk counts  201885  198691    435 400001 no-az (ref-lo) meas -0.000,009,9V mean(100) -0.0000108V, stddev(100) 0.73uV,
      clk counts  201885  198691    437 400001 no-az (ref-lo) meas -0.000,011,2V mean(100) -0.0000104V, stddev(100) 0.68uV,
      clk counts  201885  198691    437 400001 no-az (ref-lo) meas -0.000,011,2V mean(100) -0.0000110V, stddev(100) 0.72uV,


      stderr(V) 0.39uV  (nplc10)                    <- OK. best ever.  seat/ and laptop and usb core are moved
      res       0.070uV  digits 8.16   (nplc10)

      clk counts  201885  198691    438 400001 no-az (ref-lo) meas -0.000,020,9V mean(100) -0.0000207V, stddev(100) 0.78uV,
      clk counts  201885  198691    438 400001 no-az (ref-lo) meas -0.000,020,9V mean(100) -0.0000213V, stddev(100) 0.66uV,

  ------------------

  change fpga code to equalize switch counts,

  stderr(V) 0.43uV  (nplc10) res       0.062uV  digits 8.21   (nplc10)  <- good.


  gahhh. much worse??????
    clk counts  201883  198691    319 400001, reset  10002, stats 386 386 385, period 0.02, freq 19 kHz no-az (ref-lo) meas -0.000,001,3V mean(100) 0.0000004V, stddev(100) 2.25uV,

  wow.
    now ok???

    clk counts  201883  198691    319 400001 no-az (ref-lo) meas -0.000,017,9V mean(100) -0.0000181V, stddev(100) 0.61uV,
    clk counts  201883  198691    317 400001 no-az (ref-lo) meas -0.000,016,5V mean(100) -0.0000171V, stddev(100) 0.67uV,
    clk counts  201883  198691    320 400001 no-az (ref-lo) meas -0.000,018,6V mean(100) -0.0000173V, stddev(100) 0.81uV,
    clk counts  201883  198691    319 400001 no-az (ref-lo) meas -0.000,017,9V mean(100) -0.0000173V, stddev(100) 0.71uV,


OK. did another cal and it's bad again. or there is interference.
    but the offset is a lot less.

  clk counts  201884  198691    379 400001 no-az (ref-lo) meas -0.000,003,3V mean(100) -0.0000057V, stddev(100) 2.02uV,


  stderr(V) 0.42uV  (nplc10)
  res       0.069uV  digits 8.16   (nplc10)
  b row 2:   -0.275120076

  ---------------

  stderr(V) 0.23uV  (nplc10)                          OK.   absolutely best ever. (so i think having the
  res       0.066uV  digits 8.18   (nplc10)
  row 2:   -0.263541767

  ======================================
  ----------
  NO NO NO.
    - issue with apparent noise - depends on if we get equal/clean runup counts (not the calibration coefficients)
    - when redo the calibration - will affects this.
    - this is why it keeps changing,

  EXTR.   we need a processing filter - to group with the same
  ----------

  eg. when get clean runup counts - have this,

    clk counts  201884  198691    377 400001 no-az (ref-lo) meas -0.000,007,6V mean(10) -0.0000083V, stddev(10) 0.61uV,


  but mix of counts

    clk counts  201883  198691    318 400001 no-az (ref-lo) meas -0.000,012,4V mean(10) -0.0000087V, stddev(10) 1.83uV,


  variation in cal - is more a function - of changes in runup counts.

  stderr(V) 0.32uV  (nplc10) res       0.069uV  digits 8.16   (nplc10)    <- ANOTHER EXTREMELY GOOD CAL.
  stderr(V) 0.46uV  (nplc10)
  stderr(V) 0.43uV  (nplc10)


-----------------

            ref-up  ref-down  rundown.

  1nplc.
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000007V, stddev(10) 0.27uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000007V, stddev(10) 0.27uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000007V, stddev(10) 0.27uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000007V, stddev(10) 0.27uV,
  clk counts  201884  198691    380 400001 no-az (ref-lo) meas -0.000,000,0V mean(10) 0.0000006V, stddev(10) 0.36uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000007V, stddev(10) 0.40uV,
  clk counts  201884  198691    377 400001 no-az (ref-lo) meas 0.000,001,8V mean(10) 0.0000008V, stddev(10) 0.53uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000008V, stddev(10) 0.53uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000008V, stddev(10) 0.53uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000009V, stddev(10) 0.54uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000009V, stddev(10) 0.54uV,
  clk counts  201884  198691    377 400001 no-az (ref-lo) meas 0.000,001,8V mean(10) 0.0000010V, stddev(10) 0.61uV,
  clk counts  201884  198691    377 400001 no-az (ref-lo) meas 0.000,001,8V mean(10) 0.0000011V, stddev(10) 0.65uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000010V, stddev(10) 0.67uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000011V, stddev(10) 0.58uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000010V, stddev(10) 0.60uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000009V, stddev(10) 0.54uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000010V, stddev(10) 0.53uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000010V, stddev(10) 0.53uV,
  clk counts  201884  198691    380 400001 no-az (ref-lo) meas -0.000,000,0V mean(10) 0.0000008V, stddev(10) 0.61uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000008V, stddev(10) 0.61uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000008V, stddev(10) 0.52uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000006V, stddev(10) 0.36uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000007V, stddev(10) 0.40uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000008V, stddev(10) 0.43uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000008V, stddev(10) 0.43uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000008V, stddev(10) 0.44uV,
  clk counts  201884  198691    379 400001 no-az (ref-lo) meas 0.000,000,6V mean(10) 0.0000008V, stddev(10) 0.43uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000008V, stddev(10) 0.44uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000010V, stddev(10) 0.33uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000010V, stddev(10) 0.30uV,
  clk counts  201884  198691    380 400001 no-az (ref-lo) meas -0.000,000,0V mean(10) 0.0000009V, stddev(10) 0.45uV,
  clk counts  201884  198691    378 400001 no-az (ref-lo) meas 0.000,001,2V mean(10) 0.0000010V, stddev(10) 0.44uV,
  clk counts  201883  198691    316 400001 no-az (ref-lo) meas -0.000,003,4V mean(10) 0.0000005V, stddev(10) 1.44uV,    <- eg. runup counts, disturbed, increases stddev


    EXTR. NOTE also meas value is very good - and no offset.
    I think the reset approach - has a distinct advantage.
  ---

  10nplc.
  clk counts 2016138 1984299    351 4000001 no-az (ref-lo) meas 0.000,002,7V mean(10) 0.0000026V, stddev(10) 0.19uV,
  clk counts 2016138 1984299    351 4000001 no-az (ref-lo) meas 0.000,002,7V mean(10) 0.0000026V, stddev(10) 0.17uV,
  clk counts 2016138 1984299    350 4000001 no-az (ref-lo) meas 0.000,002,7V mean(10) 0.0000026V, stddev(10) 0.18uV,
  clk counts 2016138 1984299    355 4000001 no-az (ref-lo) meas 0.000,002,4V mean(10) 0.0000026V, stddev(10) 0.18uV,
  clk counts 2016138 1984299    352 4000001 no-az (ref-lo) meas 0.000,002,6V mean(10) 0.0000026V, stddev(10) 0.18uV,
  clk counts 2016138 1984299    354 4000001 no-az (ref-lo) meas 0.000,002,5V mean(10) 0.0000026V, stddev(10) 0.18uV,
  clk counts 2016138 1984299    350 4000001 no-az (ref-lo) meas 0.000,002,7V mean(10) 0.0000026V, stddev(10) 0.19uV,
  clk counts 2016138 1984299    346 4000001 no-az (ref-lo) meas 0.000,003,0V mean(10) 0.0000027V, stddev(10) 0.16uV,
  clk counts 2016138 1984299    351 4000001 no-az (ref-lo) meas 0.000,002,7V mean(10) 0.0000027V, stddev(10) 0.15uV,
  clk counts 2016138 1984299    354 4000001 no-az (ref-lo) meas 0.000,002,5V mean(10) 0.0000027V, stddev(10) 0.16uV,

  So it's enough.

  =======================

  CONCLUSION. NOISE

    we want the ability to save a cal.

    EXTR - need to use lid, and keep usb cabling.  and laptop away.  from pcb board.

    EXTR - stddev period needs to be shortish
      - to avoid thermal walk. that increases variance.
      - and changes to the runup pattern, creating changes in runup counts not just the rundown count.

    this issue - also affects the stderr() of the calibration.  and creates abrubt differences.

    Allan deviation - may be a better way to look at things.  where just use adjacent values.


    Maybe hsould have marker/ count - for number of values - since the runup count changed .



nov 28

  Using
  setting dcv-source. with noaz.

  noaz.
   reset ; dcv-source -10V ; azero off; nplc 10; himux dcv-source ; azmux pc-out ; gain 1; verbose 1; buffer 10;  trig

  az.
     reset ; dcv-source -10V ; azero on; nplc 10; himux dcv-source ; azmux ref-lo ; gain 1; verbose 1; buffer 10;  trig

  working nice.



nov 30

  reading current shunt.

  reset
  irange 1mA ;  dci-source 1mA ;  set k702 top;
  flash cal read 1;
  gain 100;
  azero off; nplc 10; himux dci-hi ; azmux pc-out ;  verbose 1; buffer 10;  trig

  works.

  reading tia.

  irange 10uA ;  dci-source 10uA ;  set k702 top;
  flash cal read 1;
  azero off; nplc 10; himux dci-tia ; azmux pc-out ; ; verbose 1; buffer 10;  trig


  no. gain appears to work.


#endif



