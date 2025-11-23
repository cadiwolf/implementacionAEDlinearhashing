"use client";

import { useState, useEffect, useCallback } from "react";

const API_URL = "http://localhost:8080";

interface Record {
  key: string;
  value: string;
}



export default function Home() {
  const [records, setRecords] = useState<Record[]>([]);
  const [searchResult, setSearchResult] = useState<Record | null>(null);

  const [message, setMessage] = useState<{ text: string; type: "success" | "error" } | null>(null);

  // Form states
  const [insertKey, setInsertKey] = useState("");
  const [insertValue, setInsertValue] = useState("");
  const [searchKey, setSearchKey] = useState("");



  const fetchRecords = useCallback(async () => {
    try {
      const res = await fetch(`${API_URL}/records`);
      if (res.ok) {
        const data = await res.json();
        // The API returns { count: n, records: [...] }
        setRecords(data.records || []);
        setMessage(null);
      } else {
        setMessage({ text: "Error fetching records", type: "error" });
      }
    } catch {
      setMessage({ text: "Network error", type: "error" });
    }
  }, []);

  // Initial load
  useEffect(() => {
    fetchRecords();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const handleInsert = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!insertKey || !insertValue) return;

    try {
      const res = await fetch(`${API_URL}/records`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ key: insertKey, value: insertValue }),
      });

      if (res.ok) {
        setMessage({ text: "Record inserted successfully", type: "success" });
        setInsertKey("");
        setInsertValue("");
        fetchRecords(); 
      } else {
        const data = await res.json();
        setMessage({ text: data.error || "Error inserting record", type: "error" });
      }
    } catch {
      setMessage({ text: "Network error", type: "error" });
    }
  };

  const handleSearch = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!searchKey) return;

    try {
      const res = await fetch(`${API_URL}/records/${encodeURIComponent(searchKey)}`);
      if (res.ok) {
        const data = await res.json();
        setSearchResult(data);
        setMessage(null);
      } else {
        setSearchResult(null);
        setMessage({ text: "Record not found", type: "error" });
      }
    } catch {
      setMessage({ text: "Network error", type: "error" });
    }
  };

  const handleDelete = async (key: string) => {
    try {
      const res = await fetch(`${API_URL}/records/${encodeURIComponent(key)}`, {
        method: "DELETE",
      });

      if (res.ok) {
        setMessage({ text: `Record ${key} deleted`, type: "success" });
        if (searchResult && searchResult.key === key) {
          setSearchResult(null);
        }
        fetchRecords(); // Refresh list if needed
      } else {
        setMessage({ text: "Error deleting record", type: "error" });
      }
    } catch {
      setMessage({ text: "Network error", type: "error" });
    }
  };

  return (
    <div className="min-h-screen bg-gray-50 p-8 font-sans text-gray-900 dark:bg-gray-900 dark:text-gray-100">
      <div className="mx-auto max-w-6xl space-y-8">
        <header className="border-b border-gray-200 pb-4 dark:border-gray-700">
          <h1 className="text-3xl font-bold text-blue-600 dark:text-blue-400">Linear Hashing Visualization</h1>
          <p className="text-gray-500 dark:text-gray-400">Interact with the C++ backend implementation</p>
        </header>

        {/* Message Banner */}
        {message && (
          <div className={`rounded-md p-4 ${message.type === 'success' ? 'bg-green-100 text-green-800 dark:bg-green-900 dark:text-green-100' : 'bg-red-100 text-red-800 dark:bg-red-900 dark:text-red-100'}`}>
            {message.text}
          </div>
        )}

        <div className="mx-auto max-w-3xl">
          {/* Controls */}
          <div className="space-y-6">
            
            {/* Insert Section */}
            <section className="rounded-lg bg-white p-6 shadow dark:bg-gray-800">
              <h2 className="mb-4 text-xl font-semibold">Insert Record</h2>
              <form onSubmit={handleInsert} className="space-y-4">
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <label className="mb-1 block text-sm font-medium">Key</label>
                    <input
                      type="text"
                      value={insertKey}
                      onChange={(e) => setInsertKey(e.target.value)}
                      className="w-full rounded-md border border-gray-300 p-2 dark:border-gray-600 dark:bg-gray-700"
                      placeholder="Enter key"
                    />
                  </div>
                  <div>
                    <label className="mb-1 block text-sm font-medium">Value</label>
                    <input
                      type="text"
                      value={insertValue}
                      onChange={(e) => setInsertValue(e.target.value)}
                      className="w-full rounded-md border border-gray-300 p-2 dark:border-gray-600 dark:bg-gray-700"
                      placeholder="Enter value"
                    />
                  </div>
                </div>
                <button
                  type="submit"
                  className="w-full rounded-md bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2"
                >
                  Insert
                </button>
              </form>
            </section>

            {/* Search Section */}
            <section className="rounded-lg bg-white p-6 shadow dark:bg-gray-800">
              <h2 className="mb-4 text-xl font-semibold">Search Record</h2>
              <form onSubmit={handleSearch} className="flex gap-2">
                <input
                  type="text"
                  value={searchKey}
                  onChange={(e) => setSearchKey(e.target.value)}
                  className="flex-1 rounded-md border border-gray-300 p-2 dark:border-gray-600 dark:bg-gray-700"
                  placeholder="Search by key"
                />
                <button
                  type="submit"
                  className="rounded-md bg-gray-600 px-4 py-2 text-white hover:bg-gray-700"
                >
                  Search
                </button>
              </form>

              {searchResult && (
                <div className="mt-4 rounded-md border border-blue-200 bg-blue-50 p-4 dark:border-blue-800 dark:bg-blue-900/30">
                  <div className="flex items-center justify-between">
                    <div>
                      <p className="font-medium">Found:</p>
                      <p>Key: <span className="font-mono font-bold">{searchResult.key}</span></p>
                      <p>Value: <span className="font-mono">{searchResult.value}</span></p>
                    </div>
                    <button
                      onClick={() => handleDelete(searchResult.key)}
                      className="rounded bg-red-500 px-3 py-1 text-sm text-white hover:bg-red-600"
                    >
                      Delete
                    </button>
                  </div>
                </div>
              )}
            </section>

            {/* List All Section */}
            <section className="rounded-lg bg-white p-6 shadow dark:bg-gray-800">
              <div className="mb-4 flex items-center justify-between">
                <h2 className="text-xl font-semibold">All Records</h2>
                <button
                  onClick={fetchRecords}
                  className="rounded-md bg-green-600 px-4 py-2 text-sm text-white hover:bg-green-700"
                >
                  Refresh List
                </button>
              </div>
              
              {records.length > 0 ? (
                <div className="max-h-60 overflow-y-auto rounded border border-gray-200 dark:border-gray-700">
                  <table className="min-w-full divide-y divide-gray-200 dark:divide-gray-700">
                    <thead className="bg-gray-50 dark:bg-gray-700">
                      <tr>
                        <th className="px-4 py-2 text-left text-xs font-medium uppercase tracking-wider text-gray-500 dark:text-gray-300">Key</th>
                        <th className="px-4 py-2 text-left text-xs font-medium uppercase tracking-wider text-gray-500 dark:text-gray-300">Value</th>
                        <th className="px-4 py-2 text-right text-xs font-medium uppercase tracking-wider text-gray-500 dark:text-gray-300">Action</th>
                      </tr>
                    </thead>
                    <tbody className="divide-y divide-gray-200 bg-white dark:divide-gray-700 dark:bg-gray-800">
                      {records.map((rec) => (
                        <tr key={rec.key}>
                          <td className="whitespace-nowrap px-4 py-2 font-mono text-sm">{rec.key}</td>
                          <td className="whitespace-nowrap px-4 py-2 text-sm">{rec.value}</td>
                          <td className="whitespace-nowrap px-4 py-2 text-right text-sm">
                            <button
                              onClick={() => handleDelete(rec.key)}
                              className="text-red-600 hover:text-red-900 dark:text-red-400 dark:hover:text-red-300"
                            >
                              Delete
                            </button>
                          </td>
                        </tr>
                      ))}
                    </tbody>
                  </table>
                </div>
              ) : (
                <p className="text-sm text-gray-500">No records loaded or empty.</p>
              )}
            </section>
          </div>


        </div>
      </div>
    </div>
  );
}
