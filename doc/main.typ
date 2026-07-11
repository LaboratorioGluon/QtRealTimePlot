#import "@preview/mmdr:0.2.2": mermaid
#let title-page(title:[], email:[],  fill: yellow, body) = {
  set page(fill: rgb("#ffc46c"), margin: (top: 1.5in, rest: 2in))
  set text(font: "Libertinus Serif", size: 14pt)
  set heading(numbering: "1.1.1")
  line(start: (0%, 0%), end: (8.5in, 0%), stroke: (thickness: 2pt))
  align(horizon + left)[
    #text(size: 24pt, title)\
    #v(1em)
    Documentation
    #v(2em)
    #link(email)
  ]
  
  align(bottom + left)[#datetime.today().display()]
  pagebreak()
  set page(fill: none, margin: auto)
  align(horizon, outline(indent: auto))
  pagebreak()
  body
}

#show: body => title-page(
  title: [RealTimePlot by LabGluon],
  email: "mailto: laboratoriogluon@gmail.com",
  body
)

#set page(margin: 1.5in)
= Introduction

Plotting library for realtime data developed for Qt 6.0.

#include "plotSeries.typ"
