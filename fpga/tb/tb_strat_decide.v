`timescale 1ns / 1ps

module tb_strat_decide;

    parameter W = 32;
    reg clk;
    reg rst;
    reg [W-1:0] bid_px0;
    reg [W-1:0] ask_px0;
    reg [W-1:0] fair_px;
    reg [W-1:0] thresh_buy;
    reg [W-1:0] thresh_sell;
    reg in_valid;

    wire buy;
    wire sell;
    wire out_valid;

    strat_decide #(W) uut (
        .clk(clk), .rst(rst),
        .bid_px0(bid_px0), .ask_px0(ask_px0),
        .fair_px(fair_px), .thresh_buy(thresh_buy), .thresh_sell(thresh_sell),
        .in_valid(in_valid),
        .buy(buy), .sell(sell), .out_valid(out_valid)
    );

    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    initial begin
        rst = 1;
        in_valid = 0;
        bid_px0 = 10000;
        ask_px0 = 10010;
        fair_px = 10005;
        thresh_buy = 5;
        thresh_sell = 5;
        
        #100;
        rst = 0;
        #20;

        // 1. Fair price is in the middle -> No action
        @(posedge clk);
        in_valid = 1;
        @(posedge clk);
        in_valid = 0;
        #10;
        if (buy == 0 && sell == 0 && out_valid == 1)
            $display("PASS: Neutral Fair Price -> No Action");
        else
            $display("FAIL: Neutral State. Buy: %b, Sell: %b", buy, sell);

        // 2. Fair price jumps high -> BUY signal
        @(posedge clk);
        fair_px = 10020; // 10010 + 5 < 10020 is true
        in_valid = 1;
        @(posedge clk);
        in_valid = 0;
        #10;
        if (buy == 1 && sell == 0)
            $display("PASS: High Fair Price -> BUY Triggered");
        else
            $display("FAIL: BUY State. Buy: %b, Sell: %b", buy, sell);

        // 3. Fair price drops low -> SELL signal
        @(posedge clk);
        fair_px = 9990; // 10000 - 5 > 9990 is true
        in_valid = 1;
        @(posedge clk);
        in_valid = 0;
        #10;
        if (buy == 0 && sell == 1)
            $display("PASS: Low Fair Price -> SELL Triggered");
        else
            $display("FAIL: SELL State. Buy: %b, Sell: %b", buy, sell);

        $finish;
    end

endmodule
