#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#include "srvrlog.h"
#include "lsconfig.h"
#include "codemap.h"
#include "nxosd.h"

/* 
 * Default ISDN cause code/SIP response code mapping and hunting configuration.
 * Original ISDN code is looked up using the index of the array.
 * Original SIP response code is looked up using an appropriate offset.
 */
CodeMap defaultcodemap[CODEMAP_ITEMS] = {
	{0, 0, 500},	// 0
	{0, 34, 404},	// 1
	{1, 34, 404},	// 2
	{1, 3, 404},	// 3
	{0, 4, 500},	// 4
	{0, 5, 500},	// 5
	{1, 6, 500},	// 6
	{0, 7, 500},	// 7
	{0, 8, 500},	// 8
	{0, 9, 500},	// 9
	{0, 10, 500},	// 10
	{0, 11, 500},	// 11
	{0, 12, 500},	// 12
	{0, 13, 500},	// 13
	{0, 14, 500},	// 14
	{0, 15, 500},	// 15
	{0, 16, 500},	// 16
	{0, 17, 486},	// 17
	{0, 34, 480},	// 18
	{0, 19, 480},	// 19
	{0, 20, 480},	// 20
	{1, 34, 403},	// 21
	{0, 22, 410},	// 22
	{0, 23, 500},	// 23
	{0, 24, 500},	// 24
	{0, 25, 500},	// 25
	{0, 26, 404},	// 26
	{1, 34, 404},	// 27
	{0, 34, 484},	// 28
	{1, 34, 501},	// 29
	{1, 30, 500},	// 30
	{1, 31, 404},	// 31
	{0, 32, 500},	// 32
	{0, 33, 500},   // 33
	{1, 34, 503},	// 34
	{0, 35, 500},	// 35
	{0, 36, 500},	// 36
	{0, 37, 500},	// 37
	{1, 38, 503},	// 38
	{0, 39, 500},	// 39
	{0, 40, 500},	// 40
	{1, 41, 503},	// 41
	{1, 42, 503},	// 42
	{1, 43, 500},	// 43
	{1, 44, 500},	// 44
	{0, 45, 500},	// 45
	{0, 46, 500},	// 46
	{1, 47, 503},	// 47
	{0, 48, 500},	// 48
	{1, 49, 500},	// 49
	{1, 34, 500},	// 50
	{0, 51, 500},	// 51
	{0, 52, 500},	// 52
	{0, 53, 500},	// 53
	{0, 54, 500},	// 54
	{0, 55, 403},	// 55
	{0, 56, 403},	// 56
	{1, 57, 403},	// 57
	{1, 58, 501},	// 58
	{0, 59, 500},	// 59
	{0, 60, 500},	// 60
	{0, 61, 500},	// 61
	{0, 62, 500},	// 62
	{1, 34, 500},	// 63
	{0, 34, 500},	// 64
	{1, 65, 501},	// 65
	{1, 66, 500},	// 66
	{0, 67, 500},	// 67
	{0, 68, 500},	// 68
	{1, 69, 500},	// 69
	{1, 70, 500},	// 70
	{0, 71, 500},	// 71
	{0, 72, 500},	// 72
	{0, 73, 500},	// 73
	{0, 74, 500},	// 74
	{0, 75, 500},	// 75
	{0, 76, 500},	// 76
	{0, 77, 500},	// 77
	{0, 78, 500},	// 78
	{1, 79, 501},	// 79
	{0, 80, 500},	// 80
	{1, 81, 500},	// 81
	{1, 82, 500},	// 82
	{1, 83, 500},	// 83
	{1, 84, 500},	// 84
	{1, 85, 500},	// 85
	{0, 86, 500},	// 86
	{0, 87, 503},	// 87
	{1, 88, 400},	// 88
	{0, 89, 500},	// 89
	{0, 90, 500},	// 90
	{1, 91, 500},	// 91
	{0, 92, 500},	// 92
	{0, 93, 500},	// 93
	{0, 94, 500},	// 94
	{1, 95, 400},	// 95
	{1, 96, 488},	// 96
	{1, 97, 500},	// 97
	{1, 98, 500},	// 98
	{1, 99, 500},	// 99
	{1, 100, 500},	// 100
	{1, 101, 500},	// 101
	{1, 102, 408},	// 102
	{0, 103, 500},	// 103
	{0, 104, 500},	// 104
	{0, 105, 500},	// 105
	{0, 106, 500},	// 106
	{0, 107, 500},	// 107
	{0, 108, 500},	// 108
	{0, 109, 500},	// 109
	{0, 110, 500},	// 110
	{1, 111, 400},	// 111
	{0, 112, 500},	// 112
	{0, 113, 500},	// 113
	{0, 114, 500},	// 114
	{0, 115, 500},	// 115
	{0, 116, 500},	// 116
	{0, 117, 500},	// 117
	{0, 118, 500},	// 118
	{0, 119, 500},	// 119
	{0, 120, 500},	// 120
	{0, 121, 500},	// 121
	{0, 122, 500},	// 122
	{0, 123, 500},	// 123
	{0, 124, 500},	// 124
	{0, 125, 500},	// 125
	{0, 126, 500},	// 126
	{1, 127, 500},	// 127
	{1, 127, 400},	// 400
	{0, 57, 401},	// 401
	{1, 21, 402},	// 402
	{0, 57, 403},	// 403
	{1, 1, 404},	// 404
	{1, 127, 405},	// 405
	{1, 127, 406},	// 406
	{0, 21, 407},	// 407
	{1, 102, 408},	// 408
	{1, 41, 409},	// 409
	{1, 1, 410},	// 410
	{1, 127, 411},	// 411
	{0, -1, 412},	// 412
	{1, 127, 413},	// 413
	{1, 127, 414},	// 414
	{1, 79, 415},	// 415
	{0, -1, 416},	// 416
	{0, -1, 417},	// 417
	{0, -1, 418},	// 418
	{0, -1, 419},	// 419
	{1, 127, 420},	// 420
	{0, -1, 421},	// 421
	{0, -1, 422},	// 422
	{0, -1, 423},	// 423
	{0, -1, 424},	// 424
	{0, -1, 425},	// 425
	{0, -1, 426},	// 426
	{0, -1, 427},	// 427
	{0, -1, 428},	// 428
	{0, -1, 429},	// 429
	{0, -1, 430},	// 430
	{0, -1, 431},	// 431
	{0, -1, 432},	// 432
	{0, -1, 433},	// 433
	{0, -1, 434},	// 434
	{0, -1, 435},	// 435
	{0, -1, 436},	// 436
	{0, -1, 437},	// 437
	{0, -1, 438},	// 438
	{0, -1, 439},	// 439
	{0, -1, 440},	// 440
	{0, -1, 441},	// 441
	{0, -1, 442},	// 442
	{0, -1, 443},	// 443
	{0, -1, 444},	// 444
	{0, -1, 445},	// 445
	{0, -1, 446},	// 446
	{0, -1, 447},	// 447
	{0, -1, 448},	// 448
	{0, -1, 449},	// 449
	{0, -1, 450},	// 450
	{0, -1, 451},	// 451
	{0, -1, 452},	// 452
	{0, -1, 453},	// 453
	{0, -1, 454},	// 454
	{0, -1, 455},	// 455
	{0, -1, 456},	// 456
	{0, -1, 457},	// 457
	{0, -1, 458},	// 458
	{0, -1, 459},	// 459
	{0, -1, 460},	// 460
	{0, -1, 461},	// 461
	{0, -1, 462},	// 462
	{0, -1, 463},	// 463
	{0, -1, 464},	// 464
	{0, -1, 465},	// 465
	{0, -1, 466},	// 466
	{0, -1, 467},	// 467
	{0, -1, 468},	// 468
	{0, -1, 469},	// 469
	{0, -1, 470},	// 470
	{0, -1, 471},	// 471
	{0, -1, 472},	// 472
	{0, -1, 473},	// 473
	{0, -1, 474},	// 474
	{0, -1, 475},	// 475
	{0, -1, 476},	// 476
	{0, -1, 477},	// 477
	{0, -1, 478},	// 478
	{0, -1, 479},	// 479
	{1, 18, 480},	// 480
	{1, 127, 481},	// 481
	{1, 127, 482},	// 482
	{1, 127, 483},	// 483
	{1, 28, 484},	// 484
	{1, 1, 485},	// 485
	{0, 17, 486},	// 486
	{1, 127, 487},	// 487
	{1, 127, 488},	// 488
	{0, -1, 489},	// 489
	{0, -1, 490},	// 490
	{0, -1, 491},	// 491
	{0, -1, 492},	// 492
	{0, -1, 493},	// 493
	{0, -1, 494},	// 494
	{0, -1, 495},	// 495
	{0, -1, 496},	// 496
	{0, -1, 497},	// 497
	{0, -1, 498},	// 498
	{0, -1, 499},	// 499
	{1, 41, 500},	// 500
	{0, 79, 501},	// 501
	{1, 38, 502},	// 502
	{1, 63, 503},	// 503
	{1, 102, 504},	// 504
	{1, 127, 505},	// 505
	{0, -1, 506},	// 506
	{0, -1, 507},	// 507
	{0, -1, 508},	// 508
	{0, -1, 509},	// 509
	{0, -1, 510},	// 510
	{0, -1, 511},	// 511
	{0, -1, 512},	// 512
	{0, -1, 513},	// 513
	{0, -1, 514},	// 514
	{0, -1, 515},	// 515
	{0, -1, 516},	// 516
	{0, -1, 517},	// 517
	{0, -1, 518},	// 518
	{0, -1, 519},	// 519
	{0, -1, 520},	// 520
	{0, -1, 521},	// 521
	{0, -1, 522},	// 522
	{0, -1, 523},	// 523
	{0, -1, 524},	// 524
	{0, -1, 525},	// 525
	{0, -1, 526},	// 526
	{0, -1, 527},	// 527
	{0, -1, 528},	// 528
	{0, -1, 529},	// 529
	{0, -1, 530},	// 530
	{0, -1, 531},	// 531
	{0, -1, 532},	// 532
	{0, -1, 533},	// 533
	{0, -1, 534},	// 534
	{0, -1, 535},	// 535
	{0, -1, 536},	// 536
	{0, -1, 537},	// 537
	{0, -1, 538},	// 538
	{0, -1, 539},	// 539
	{0, -1, 540},	// 540
	{0, -1, 541},	// 541
	{0, -1, 542},	// 542
	{0, -1, 543},	// 543
	{0, -1, 544},	// 544
	{0, -1, 545},	// 545
	{0, -1, 546},	// 546
	{0, -1, 547},	// 547
	{0, -1, 548},	// 548
	{0, -1, 549},	// 549
	{0, -1, 550},	// 550
	{0, -1, 551},	// 551
	{0, -1, 552},	// 552
	{0, -1, 553},	// 553
	{0, -1, 554},	// 554
	{0, -1, 555},	// 555
	{0, -1, 556},	// 556
	{0, -1, 557},	// 557
	{0, -1, 558},	// 558
	{0, -1, 559},	// 559
	{0, -1, 560},	// 560
	{0, -1, 561},	// 561
	{0, -1, 562},	// 562
	{0, -1, 563},	// 563
	{0, -1, 564},	// 564
	{0, -1, 565},	// 565
	{0, -1, 566},	// 566
	{0, -1, 567},	// 567
	{0, -1, 568},	// 568
	{0, -1, 569},	// 569
	{0, -1, 570},	// 570
	{0, -1, 571},	// 571
	{0, -1, 572},	// 572
	{0, -1, 573},	// 573
	{0, -1, 574},	// 574
	{0, -1, 575},	// 575
	{0, -1, 576},	// 576
	{0, -1, 577},	// 577
	{0, -1, 578},	// 578
	{0, -1, 579},	// 579
	{1, 47, 580},	// 580
	{0, -1, 581},	// 581
	{0, -1, 582},	// 582
	{0, -1, 583},	// 583
	{0, -1, 584},	// 584
	{0, -1, 585},	// 585
	{0, -1, 586},	// 586
	{0, -1, 587},	// 587
	{0, -1, 588},	// 588
	{0, -1, 589},	// 589
	{0, -1, 590},	// 590
	{0, -1, 591},	// 591
	{0, -1, 592},	// 592
	{0, -1, 593},	// 593
	{0, -1, 594},	// 594
	{0, -1, 595},	// 595
	{0, -1, 596},	// 596
	{0, -1, 597},	// 597
	{0, -1, 598},	// 598
	{0, -1, 599},	// 599
	{1, 17, 600},	// 600
	{0, -1, 601},	// 601
	{0, -1, 602},	// 602
	{1, 21, 603},	// 603
	{0, -1, 604},	// 604
	{0, -1, 605},	// 605
	{1, 58, 606},	// 606
	{0, -1, 607},	// 607
	{0, -1, 608},	// 608
	{0, -1, 609},	// 609
	{0, -1, 610},	// 610
	{0, -1, 611},	// 611
	{0, -1, 612},	// 612
	{0, -1, 613},	// 613
	{0, -1, 614},	// 614
	{0, -1, 615},	// 615
	{0, -1, 616},	// 616
	{0, -1, 617},	// 617
	{0, -1, 618},	// 618
	{0, -1, 619},	// 619
	{0, -1, 620},	// 620
	{0, -1, 621},	// 621
	{0, -1, 622},	// 622
	{0, -1, 623},	// 623
	{0, -1, 624},	// 624
	{0, -1, 625},	// 625
	{0, -1, 626},	// 626
	{0, -1, 627},	// 627
	{0, -1, 628},	// 628
	{0, -1, 629},	// 629
	{0, -1, 630},	// 630
	{0, -1, 631},	// 631
	{0, -1, 632},	// 632
	{0, -1, 633},	// 633
	{0, -1, 634},	// 634
	{0, -1, 635},	// 635
	{0, -1, 636},	// 636
	{0, -1, 637},	// 637
	{0, -1, 638},	// 638
	{0, -1, 639},	// 639
	{0, -1, 640},	// 640
	{0, -1, 641},	// 641
	{0, -1, 642},	// 642
	{0, -1, 643},	// 643
	{0, -1, 644},	// 644
	{0, -1, 645},	// 645
	{0, -1, 646},	// 646
	{0, -1, 647},	// 647
	{0, -1, 648},	// 648
	{0, -1, 649},	// 649
	{0, -1, 650},	// 650
	{0, -1, 651},	// 651
	{0, -1, 652},	// 652
	{0, -1, 653},	// 653
	{0, -1, 654},	// 654
	{0, -1, 655},	// 655
	{0, -1, 656},	// 656
	{0, -1, 657},	// 657
	{0, -1, 658},	// 658
	{0, -1, 659},	// 659
	{0, -1, 660},	// 660
	{0, -1, 661},	// 661
	{0, -1, 662},	// 662
	{0, -1, 663},	// 663
	{0, -1, 664},	// 664
	{0, -1, 665},	// 665
	{0, -1, 666},	// 666
	{0, -1, 667},	// 667
	{0, -1, 668},	// 668
	{0, -1, 669},	// 669
	{0, -1, 670},	// 670
	{0, -1, 671},	// 671
	{0, -1, 672},	// 672
	{0, -1, 673},	// 673
	{0, -1, 674},	// 674
	{0, -1, 675},	// 675
	{0, -1, 676},	// 676
	{0, -1, 677},	// 677
	{0, -1, 678},	// 678
	{0, -1, 679},	// 679
	{0, -1, 680},	// 680
	{0, -1, 681},	// 681
	{0, -1, 682},	// 682
	{0, -1, 683},	// 683
	{0, -1, 684},	// 684
	{0, -1, 685},	// 685
	{0, -1, 686},	// 686
	{0, -1, 687},	// 687
	{0, -1, 688},	// 688
	{0, -1, 689},	// 689
	{0, -1, 690},	// 690
	{0, -1, 691},	// 691
	{0, -1, 692},	// 692
	{0, -1, 693},	// 693
	{0, -1, 694},	// 694
	{0, -1, 695},	// 695
	{0, -1, 696},	// 696
	{0, -1, 697},	// 697
	{0, -1, 698},	// 698
	{0, -1, 699}	// 699
};

/* 
 * Configurable ISDN cause code/SIP response code mapping and hunting array
 */
CodeMap codemap[CODEMAP_ITEMS] = {
	{0, 0, 500},	// 0
	{0, 34, 404},	// 1
	{1, 34, 404},	// 2
	{1, 3, 404},	// 3
	{0, 4, 500},	// 4
	{0, 5, 500},	// 5
	{1, 6, 500},	// 6
	{0, 7, 500},	// 7
	{0, 8, 500},	// 8
	{0, 9, 500},	// 9
	{0, 10, 500},	// 10
	{0, 11, 500},	// 11
	{0, 12, 500},	// 12
	{0, 13, 500},	// 13
	{0, 14, 500},	// 14
	{0, 15, 500},	// 15
	{0, 16, 500},	// 16
	{0, 17, 486},	// 17
	{0, 34, 480},	// 18
	{0, 19, 480},	// 19
	{0, 20, 480},	// 20
	{1, 34, 403},	// 21
	{0, 22, 410},	// 22
	{0, 23, 500},	// 23
	{0, 24, 500},	// 24
	{0, 25, 500},	// 25
	{0, 26, 404},	// 26
	{1, 34, 404},	// 27
	{0, 34, 484},	// 28
	{1, 34, 501},	// 29
	{1, 30, 500},	// 30
	{1, 31, 404},	// 31
	{0, 32, 500},	// 32
	{0, 33, 500},   // 33
	{1, 34, 503},	// 34
	{0, 35, 500},	// 35
	{0, 36, 500},	// 36
	{0, 37, 500},	// 37
	{1, 38, 503},	// 38
	{0, 39, 500},	// 39
	{0, 40, 500},	// 40
	{1, 41, 503},	// 41
	{1, 42, 503},	// 42
	{1, 43, 500},	// 43
	{1, 44, 500},	// 44
	{0, 45, 500},	// 45
	{0, 46, 500},	// 46
	{1, 47, 503},	// 47
	{0, 48, 500},	// 48
	{1, 49, 500},	// 49
	{1, 34, 500},	// 50
	{0, 51, 500},	// 51
	{0, 52, 500},	// 52
	{0, 53, 500},	// 53
	{0, 54, 500},	// 54
	{0, 55, 403},	// 55
	{0, 56, 403},	// 56
	{1, 57, 403},	// 57
	{1, 58, 501},	// 58
	{0, 59, 500},	// 59
	{0, 60, 500},	// 60
	{0, 61, 500},	// 61
	{0, 62, 500},	// 62
	{1, 34, 500},	// 63
	{0, 34, 500},	// 64
	{1, 65, 501},	// 65
	{1, 66, 500},	// 66
	{0, 67, 500},	// 67
	{0, 68, 500},	// 68
	{1, 69, 500},	// 69
	{1, 70, 500},	// 70
	{0, 71, 500},	// 71
	{0, 72, 500},	// 72
	{0, 73, 500},	// 73
	{0, 74, 500},	// 74
	{0, 75, 500},	// 75
	{0, 76, 500},	// 76
	{0, 77, 500},	// 77
	{0, 78, 500},	// 78
	{1, 79, 501},	// 79
	{0, 80, 500},	// 80
	{1, 81, 500},	// 81
	{1, 82, 500},	// 82
	{1, 83, 500},	// 83
	{1, 84, 500},	// 84
	{1, 85, 500},	// 85
	{0, 86, 500},	// 86
	{0, 87, 503},	// 87
	{1, 88, 400},	// 88
	{0, 89, 500},	// 89
	{0, 90, 500},	// 90
	{1, 91, 500},	// 91
	{0, 92, 500},	// 92
	{0, 93, 500},	// 93
	{0, 94, 500},	// 94
	{1, 95, 400},	// 95
	{1, 96, 488},	// 96
	{1, 97, 500},	// 97
	{1, 98, 500},	// 98
	{1, 99, 500},	// 99
	{1, 100, 500},	// 100
	{1, 101, 500},	// 101
	{1, 102, 408},	// 102
	{0, 103, 500},	// 103
	{0, 104, 500},	// 104
	{0, 105, 500},	// 105
	{0, 106, 500},	// 106
	{0, 107, 500},	// 107
	{0, 108, 500},	// 108
	{0, 109, 500},	// 109
	{0, 110, 500},	// 110
	{1, 111, 400},	// 111
	{0, 112, 500},	// 112
	{0, 113, 500},	// 113
	{0, 114, 500},	// 114
	{0, 115, 500},	// 115
	{0, 116, 500},	// 116
	{0, 117, 500},	// 117
	{0, 118, 500},	// 118
	{0, 119, 500},	// 119
	{0, 120, 500},	// 120
	{0, 121, 500},	// 121
	{0, 122, 500},	// 122
	{0, 123, 500},	// 123
	{0, 124, 500},	// 124
	{0, 125, 500},	// 125
	{0, 126, 500},	// 126
	{1, 127, 500},	// 127
	{1, 127, 400},	// 400
	{0, 57, 401},	// 401
	{1, 21, 402},	// 402
	{0, 57, 403},	// 403
	{1, 1, 404},	// 404
	{1, 127, 405},	// 405
	{1, 127, 406},	// 406
	{0, 21, 407},	// 407
	{1, 102, 408},	// 408
	{1, 41, 409},	// 409
	{1, 1, 410},	// 410
	{1, 127, 411},	// 411
	{0, -1, 412},	// 412
	{1, 127, 413},	// 413
	{1, 127, 414},	// 414
	{1, 79, 415},	// 415
	{0, -1, 416},	// 416
	{0, -1, 417},	// 417
	{0, -1, 418},	// 418
	{0, -1, 419},	// 419
	{1, 127, 420},	// 420
	{0, -1, 421},	// 421
	{0, -1, 422},	// 422
	{0, -1, 423},	// 423
	{0, -1, 424},	// 424
	{0, -1, 425},	// 425
	{0, -1, 426},	// 426
	{0, -1, 427},	// 427
	{0, -1, 428},	// 428
	{0, -1, 429},	// 429
	{0, -1, 430},	// 430
	{0, -1, 431},	// 431
	{0, -1, 432},	// 432
	{0, -1, 433},	// 433
	{0, -1, 434},	// 434
	{0, -1, 435},	// 435
	{0, -1, 436},	// 436
	{0, -1, 437},	// 437
	{0, -1, 438},	// 438
	{0, -1, 439},	// 439
	{0, -1, 440},	// 440
	{0, -1, 441},	// 441
	{0, -1, 442},	// 442
	{0, -1, 443},	// 443
	{0, -1, 444},	// 444
	{0, -1, 445},	// 445
	{0, -1, 446},	// 446
	{0, -1, 447},	// 447
	{0, -1, 448},	// 448
	{0, -1, 449},	// 449
	{0, -1, 450},	// 450
	{0, -1, 451},	// 451
	{0, -1, 452},	// 452
	{0, -1, 453},	// 453
	{0, -1, 454},	// 454
	{0, -1, 455},	// 455
	{0, -1, 456},	// 456
	{0, -1, 457},	// 457
	{0, -1, 458},	// 458
	{0, -1, 459},	// 459
	{0, -1, 460},	// 460
	{0, -1, 461},	// 461
	{0, -1, 462},	// 462
	{0, -1, 463},	// 463
	{0, -1, 464},	// 464
	{0, -1, 465},	// 465
	{0, -1, 466},	// 466
	{0, -1, 467},	// 467
	{0, -1, 468},	// 468
	{0, -1, 469},	// 469
	{0, -1, 470},	// 470
	{0, -1, 471},	// 471
	{0, -1, 472},	// 472
	{0, -1, 473},	// 473
	{0, -1, 474},	// 474
	{0, -1, 475},	// 475
	{0, -1, 476},	// 476
	{0, -1, 477},	// 477
	{0, -1, 478},	// 478
	{0, -1, 479},	// 479
	{1, 18, 480},	// 480
	{1, 127, 481},	// 481
	{1, 127, 482},	// 482
	{1, 127, 483},	// 483
	{1, 28, 484},	// 484
	{1, 1, 485},	// 485
	{0, 17, 486},	// 486
	{1, 127, 487},	// 487
	{1, 127, 488},	// 488
	{0, -1, 489},	// 489
	{0, -1, 490},	// 490
	{0, -1, 491},	// 491
	{0, -1, 492},	// 492
	{0, -1, 493},	// 493
	{0, -1, 494},	// 494
	{0, -1, 495},	// 495
	{0, -1, 496},	// 496
	{0, -1, 497},	// 497
	{0, -1, 498},	// 498
	{0, -1, 499},	// 499
	{1, 41, 500},	// 500
	{0, 79, 501},	// 501
	{1, 38, 502},	// 502
	{1, 63, 503},	// 503
	{1, 102, 504},	// 504
	{1, 127, 505},	// 505
	{0, -1, 506},	// 506
	{0, -1, 507},	// 507
	{0, -1, 508},	// 508
	{0, -1, 509},	// 509
	{0, -1, 510},	// 510
	{0, -1, 511},	// 511
	{0, -1, 512},	// 512
	{0, -1, 513},	// 513
	{0, -1, 514},	// 514
	{0, -1, 515},	// 515
	{0, -1, 516},	// 516
	{0, -1, 517},	// 517
	{0, -1, 518},	// 518
	{0, -1, 519},	// 519
	{0, -1, 520},	// 520
	{0, -1, 521},	// 521
	{0, -1, 522},	// 522
	{0, -1, 523},	// 523
	{0, -1, 524},	// 524
	{0, -1, 525},	// 525
	{0, -1, 526},	// 526
	{0, -1, 527},	// 527
	{0, -1, 528},	// 528
	{0, -1, 529},	// 529
	{0, -1, 530},	// 530
	{0, -1, 531},	// 531
	{0, -1, 532},	// 532
	{0, -1, 533},	// 533
	{0, -1, 534},	// 534
	{0, -1, 535},	// 535
	{0, -1, 536},	// 536
	{0, -1, 537},	// 537
	{0, -1, 538},	// 538
	{0, -1, 539},	// 539
	{0, -1, 540},	// 540
	{0, -1, 541},	// 541
	{0, -1, 542},	// 542
	{0, -1, 543},	// 543
	{0, -1, 544},	// 544
	{0, -1, 545},	// 545
	{0, -1, 546},	// 546
	{0, -1, 547},	// 547
	{0, -1, 548},	// 548
	{0, -1, 549},	// 549
	{0, -1, 550},	// 550
	{0, -1, 551},	// 551
	{0, -1, 552},	// 552
	{0, -1, 553},	// 553
	{0, -1, 554},	// 554
	{0, -1, 555},	// 555
	{0, -1, 556},	// 556
	{0, -1, 557},	// 557
	{0, -1, 558},	// 558
	{0, -1, 559},	// 559
	{0, -1, 560},	// 560
	{0, -1, 561},	// 561
	{0, -1, 562},	// 562
	{0, -1, 563},	// 563
	{0, -1, 564},	// 564
	{0, -1, 565},	// 565
	{0, -1, 566},	// 566
	{0, -1, 567},	// 567
	{0, -1, 568},	// 568
	{0, -1, 569},	// 569
	{0, -1, 570},	// 570
	{0, -1, 571},	// 571
	{0, -1, 572},	// 572
	{0, -1, 573},	// 573
	{0, -1, 574},	// 574
	{0, -1, 575},	// 575
	{0, -1, 576},	// 576
	{0, -1, 577},	// 577
	{0, -1, 578},	// 578
	{0, -1, 579},	// 579
	{1, 47, 580},	// 580
	{0, -1, 581},	// 581
	{0, -1, 582},	// 582
	{0, -1, 583},	// 583
	{0, -1, 584},	// 584
	{0, -1, 585},	// 585
	{0, -1, 586},	// 586
	{0, -1, 587},	// 587
	{0, -1, 588},	// 588
	{0, -1, 589},	// 589
	{0, -1, 590},	// 590
	{0, -1, 591},	// 591
	{0, -1, 592},	// 592
	{0, -1, 593},	// 593
	{0, -1, 594},	// 594
	{0, -1, 595},	// 595
	{0, -1, 596},	// 596
	{0, -1, 597},	// 597
	{0, -1, 598},	// 598
	{0, -1, 599},	// 599
	{1, 17, 600},	// 600
	{0, -1, 601},	// 601
	{0, -1, 602},	// 602
	{1, 21, 603},	// 603
	{0, -1, 604},	// 604
	{0, -1, 605},	// 605
	{1, 58, 606},	// 606
	{0, -1, 607},	// 607
	{0, -1, 608},	// 608
	{0, -1, 609},	// 609
	{0, -1, 610},	// 610
	{0, -1, 611},	// 611
	{0, -1, 612},	// 612
	{0, -1, 613},	// 613
	{0, -1, 614},	// 614
	{0, -1, 615},	// 615
	{0, -1, 616},	// 616
	{0, -1, 617},	// 617
	{0, -1, 618},	// 618
	{0, -1, 619},	// 619
	{0, -1, 620},	// 620
	{0, -1, 621},	// 621
	{0, -1, 622},	// 622
	{0, -1, 623},	// 623
	{0, -1, 624},	// 624
	{0, -1, 625},	// 625
	{0, -1, 626},	// 626
	{0, -1, 627},	// 627
	{0, -1, 628},	// 628
	{0, -1, 629},	// 629
	{0, -1, 630},	// 630
	{0, -1, 631},	// 631
	{0, -1, 632},	// 632
	{0, -1, 633},	// 633
	{0, -1, 634},	// 634
	{0, -1, 635},	// 635
	{0, -1, 636},	// 636
	{0, -1, 637},	// 637
	{0, -1, 638},	// 638
	{0, -1, 639},	// 639
	{0, -1, 640},	// 640
	{0, -1, 641},	// 641
	{0, -1, 642},	// 642
	{0, -1, 643},	// 643
	{0, -1, 644},	// 644
	{0, -1, 645},	// 645
	{0, -1, 646},	// 646
	{0, -1, 647},	// 647
	{0, -1, 648},	// 648
	{0, -1, 649},	// 649
	{0, -1, 650},	// 650
	{0, -1, 651},	// 651
	{0, -1, 652},	// 652
	{0, -1, 653},	// 653
	{0, -1, 654},	// 654
	{0, -1, 655},	// 655
	{0, -1, 656},	// 656
	{0, -1, 657},	// 657
	{0, -1, 658},	// 658
	{0, -1, 659},	// 659
	{0, -1, 660},	// 660
	{0, -1, 661},	// 661
	{0, -1, 662},	// 662
	{0, -1, 663},	// 663
	{0, -1, 664},	// 664
	{0, -1, 665},	// 665
	{0, -1, 666},	// 666
	{0, -1, 667},	// 667
	{0, -1, 668},	// 668
	{0, -1, 669},	// 669
	{0, -1, 670},	// 670
	{0, -1, 671},	// 671
	{0, -1, 672},	// 672
	{0, -1, 673},	// 673
	{0, -1, 674},	// 674
	{0, -1, 675},	// 675
	{0, -1, 676},	// 676
	{0, -1, 677},	// 677
	{0, -1, 678},	// 678
	{0, -1, 679},	// 679
	{0, -1, 680},	// 680
	{0, -1, 681},	// 681
	{0, -1, 682},	// 682
	{0, -1, 683},	// 683
	{0, -1, 684},	// 684
	{0, -1, 685},	// 685
	{0, -1, 686},	// 686
	{0, -1, 687},	// 687
	{0, -1, 688},	// 688
	{0, -1, 689},	// 689
	{0, -1, 690},	// 690
	{0, -1, 691},	// 691
	{0, -1, 692},	// 692
	{0, -1, 693},	// 693
	{0, -1, 694},	// 694
	{0, -1, 695},	// 695
	{0, -1, 696},	// 696
	{0, -1, 697},	// 697
	{0, -1, 698},	// 698
	{0, -1, 699}	// 699
};

/* Print code configuration */
static int
printCodeMap()
{
	int i, j;

	/* Print ISDN code configuration */
  	NETDEBUG(MINIT, NETLOG_DEBUG4, 
		("MAPPING AND HUNTING CONFIG FOR ISDN CAUSE CODES\n"));
  	NETDEBUG(MINIT, NETLOG_DEBUG4, 
		("IsdnCode Hunt NewIsdnCode NewSipCode\n"));
	for (i = 0; i < 128; i++)
	{
  		NETDEBUG(MINIT, NETLOG_DEBUG4, 
			("\t%d\t\t%s\t\t%d\t\t%d\n", 
			i, (codemap[i].hunt == 1) ? "yes" : "no", 
			codemap[i].isdncode, codemap[i].sipcode));
	}

	/* Print SIP code configuration */
  	NETDEBUG(MINIT, NETLOG_DEBUG4, 
		("MAPPING AND HUNTING CONFIG FOR SIP RESPONSE CODES\n"));
  	NETDEBUG(MINIT, NETLOG_DEBUG4, 
		("SipCode Hunt NewIsdnCode NewSipCode\n"));
	for (i = 400; i < 700; i++)
	{
		j = CODEMAP_SIPINDEX(i);
  		NETDEBUG(MINIT, NETLOG_DEBUG4, 
			("\t%d\t\t%s\t\t%d\t\t%d\n", 
			i, (codemap[j].hunt == 1) ? "yes" : "no", 
			codemap[j].isdncode, codemap[j].sipcode));
	}

	return 0;
}

/* Read the configuration from codemap file into array */
static int
populateCodeMap()
{
	char filename[256] = "/usr/local/nextone/bin/";
  	char *path = getenv("SERPLEXPATH");
  	FILE *file = NULL;
  	int i, j, pos;
	int32_t checksum, newchecksum = 0;
	char *p = NULL;

	/* Check format of filename */
	if (strlen(codemapfilename) != strlen("codemap_xxx.dat"))
	{
    	NETERROR(MINIT, ("Codemap filename must be of the form codemap_xxx.dat, where xxx must be a number between 001 and 999\n"));
		goto _error;
	}

	for (i = 0, p = codemapfilename + strlen("codemap_"); i < 3; i++)
	{
		if (!isdigit(p[i]))
		{
    		NETERROR(MINIT, ("Codemap filename must be of the form codemap_xxx.dat, where xxx must be a number between 001 and 999\n"));
			goto _error;
		}
	}

  	/* get codemap filename */ 
  	if (path)
    	strcpy(filename, path);
  	nx_strlcat(filename, codemapfilename, sizeof(filename));

  	/* open codemap file */
  	if ((file = fopen(filename, "r")) == NULL)
  	{
    	NETERROR(MINIT, ("Cannot open codemap file %s - %s\n", 
	  		filename, strerror(errno)));
		goto _error;
  	}

	/* read configuration from codemap file into codemap array */ 
  	if (fread(codemap, CODEMAP_SIZE, CODEMAP_ITEMS, file) != CODEMAP_ITEMS)
  	{
    	NETERROR(MINIT, 
			("Error while reading codemap file %s\n", filename));	
		goto _error;
  	}

	/* read checksum from codemap file */ 
    if (fread(&checksum, sizeof(int), 1, file) != 1)
    {
    	NETERROR(MINIT, 
			("Error while reading checksum from file %s\n", filename));	
		goto _error;
    }
    
    for (i = 0; i < CODEMAP_ITEMS; i++)
    {
		/* recalculate the checksum */
		/* each byte in the binary file is XORed with its position in */
        /* the file and the checksum is incremented with the result */
        for (j = 0, pos = i * CODEMAP_SIZE; j < CODEMAP_SIZE; j++, pos++)
            newchecksum += (((char *) &codemap[i])[j] ^ pos);

		/* convert SIP code to host order */
        codemap[i].sipcode = ntohs(codemap[i].sipcode);
    }

    newchecksum = htonl(newchecksum);	/* convert to network order */

	/* compare checksums */
    if (newchecksum != checksum)
    {
    	NETERROR(MINIT, ("Checksum error in file %s\n", filename));	
		goto _error;
    }

  	NETDEBUG(MINIT, NETLOG_DEBUG4, 
		("Code mapping and hunting configuration based on %s\n", filename));

  	/* print codemap */
	printCodeMap();

  	/* close codemap file */
  	fclose(file);

  	return 0;

_error:
   	NETERROR(MINIT, 
		("Using default code mapping and hunting configuration\n"));
	memcpy(codemap, defaultcodemap, CODEMAP_ITEMS * CODEMAP_SIZE);
   	if (file)
		fclose(file);
	printCodeMap();
   	return -1;
}

/* Configure ISDN cause code and SIP response code mapping */
int
CodeMapConfig()
{
  return populateCodeMap();
}

/* Reconfigure ISDN cause code and SIP response code mapping */
int
CodeMapReconfig()
{
  return populateCodeMap();
}