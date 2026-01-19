`timescale 1ns / 1ps

module tx_bridge (
    input wire clk,
    input wire rst,
    
    // Input from Encoder
    input wire [63:0] enc_tdata,
    input wire enc_tvalid,
    input wire enc_tlast,
    output wire enc_tready,
    
    // Output to Network MAC
    output wire [63:0] mac_tdata,
    output wire mac_tvalid,
    output wire mac_tlast,
    input wire mac_tready
);

    // Simple pass-through bridge for protoyping
    // In a production system, this would handle CDC (Clock Domain Crossing)
    // or rate-limiting.
    
    assign mac_tdata  = enc_tdata;
    assign mac_tvalid = enc_tvalid;
    assign mac_tlast  = enc_tlast;
    assign enc_tready = mac_tready;

endmodule
