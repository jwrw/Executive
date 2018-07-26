# Executive

An Arduino library to run tasks on a regular, timed basis.

There seem to be a few similar libraries, but none seemed to be quite right for what I wanted.

See the Examples for how to use but here's a summary.

a) Set up a list of all the functions that you want called and the time schedule on which you want them to be called.  I was really thinking about tasks that need to be run every few ms, or possibly every few seconds.  The timing will not be exact, as it relies on your code calling the scheduler on a regular basis and on your routines not taking a lot of time.  (e.g. if you have a routine that needs to be called every 20ms but you have another routine called every 1000ms but that runs for 500ms then the 20ms routine will not get called every 20ms for about half the time)

b) Call the Exec.delay() or Exec.run() functions on a regular basis (as often as you can)

c) The appropriate scheduled functions will be called during your Exec.delay() or Exec.yield() functions.

d) Enjoy

In use, it soon became clear that a few other functions would be useful. So the library now allows tasks to be enabled/disabled, deleted and 'one-shot' tasks that
delete themselves after use.

If you write your entire sketch as a series of routines called by Exec then you can hand over control to Exec
entirely with a call to Exec.loop(). In the simplest case, your Arduino sketch loop() function then becomes...
```
loop() {
  Exec.loop();
}
```

HTML documentation will be found in the docs folder of the library.
