// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexLoopDetect monitors convex hull creation and aborts it on loop
// Created to detect (now fixed) bug, probably unnecessary

#ifndef CONVEX_LOOP_DETECT_H
#define CONVEX_LOOP_DETECT_H

struct convexFig;

// Resets and enables loop detection
extern void convexLoopDetectReset();

// Disables loop detection
extern void convexLoopDetectDisable();

// Monitored action (parent-child attach/detach) event
extern void convexLoopDetectAction(struct convexFig *parent, struct convexFig *child);

#endif
